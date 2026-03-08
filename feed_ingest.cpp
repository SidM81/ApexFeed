#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <cerrno>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Core/MessageStats.h"
#include "Core/SpscRingBuffer.h"
#include "OrderBook/OrderBook.h"
#include "Parser/ITCH.h"
#include "Socket/Socket.h"

namespace {

constexpr std::size_t kRecvBufferSize = 65536;
constexpr std::size_t kChunkPayloadSize = 8192;
constexpr std::size_t kRingCapacity = 1024;
constexpr int kEpollMaxEvents = 64;

struct FeedChunk {
    std::array<std::uint8_t, kChunkPayloadSize> data{};
    std::uint16_t len{0};
};

using Ring = apex::SpscRingBuffer<FeedChunk, kRingCapacity>;

std::atomic<bool> g_running{true};

void handleSignal(int) {
    g_running.store(false, std::memory_order_relaxed);
}

bool addFdToEpoll(int epfd, int fd, std::uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    return ::epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == 0;
}

void removeFdFromEpoll(int epfd, int fd) {
    ::epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
}

char messageType(const Field* fields, std::size_t count) {
    if (count == 0 || fields[0].type != TYPE::MSGTYPE || fields[0].ascii_len == 0) {
        return '?';
    }
    return fields[0].ascii[0];
}

SocketConfig parseConfig(int argc, char** argv, std::uint64_t& reportEvery) {
    SocketConfig cfg{};
    cfg.type = SocketType::TCP;
    cfg.role = SocketRole::SERVER;
    cfg.ip = "0.0.0.0";
    cfg.port = 9001;
    cfg.non_blocking = true;
    cfg.reuse_addr = true;
    cfg.reuse_port = true;

    reportEvery = 100000;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        if (arg.rfind("--mode=", 0) == 0) {
            const std::string mode = arg.substr(7);
            if (mode == "tcp") {
                cfg.type = SocketType::TCP;
            } else if (mode == "udp") {
                cfg.type = SocketType::UDP;
            } else if (mode == "multicast") {
                cfg.type = SocketType::UDP_MULTICAST;
            }
            continue;
        }

        if (arg.rfind("--ip=", 0) == 0) {
            cfg.ip = arg.substr(5);
            continue;
        }

        if (arg.rfind("--iface=", 0) == 0) {
            cfg.multicast_interface = arg.substr(8);
            continue;
        }

        if (arg.rfind("--port=", 0) == 0) {
            cfg.port = static_cast<std::uint16_t>(std::stoi(arg.substr(7)));
            continue;
        }

        if (arg == "--blocking") {
            cfg.non_blocking = false;
            continue;
        }

        if (arg.rfind("--report=", 0) == 0) {
            reportEvery = static_cast<std::uint64_t>(std::stoull(arg.substr(9)));
            continue;
        }
    }

    return cfg;
}

}  // namespace

int main(int argc, char** argv) {
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    std::uint64_t reportEvery = 0;
    SocketConfig cfg = parseConfig(argc, argv, reportEvery);

    Socket ingress(cfg);
    if (!ingress.open()) {
        std::cerr << "failed to open ingress socket\n";
        return 1;
    }

    const int epfd = ::epoll_create1(0);
    if (epfd < 0) {
        std::cerr << "epoll_create1 failed\n";
        return 1;
    }

    if (!addFdToEpoll(epfd, ingress.fd(), EPOLLIN | EPOLLERR | EPOLLHUP)) {
        std::cerr << "failed to add ingress fd to epoll\n";
        ::close(epfd);
        return 1;
    }

    auto ring = std::make_unique<Ring>();
    Ring* ringPtr = ring.get();

    std::atomic<std::uint64_t> chunkBackpressure{0};

    std::thread parserThread([&]() {
        ITCH parser;
        apex::OrderBook book;
        apex::MessageStats stats(reportEvery);

        while (g_running.load(std::memory_order_relaxed) || ringPtr->size() > 0) {
            FeedChunk chunk{};
            if (!ringPtr->tryPop(chunk)) {
                std::this_thread::yield();
                continue;
            }

            std::array<std::uint64_t, 256> localCounts{};
            std::uint64_t parsedMessages = 0;
            auto parseStart = std::chrono::steady_clock::now();

            parser.processBuffer(chunk.data.data(), chunk.len, [&](const Field* fields, std::size_t count) {
                const char type = messageType(fields, count);
                ++localCounts[static_cast<unsigned char>(type)];
                ++parsedMessages;
                book.applyITCH(fields, count);
            });

            auto parseEnd = std::chrono::steady_clock::now();
            const auto parseNs = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(parseEnd - parseStart).count());

            stats.onBatch(localCounts, parsedMessages, parseNs);
        }

        stats.report(true);
        stats.printTypeBreakdown();

        const auto tob = book.topOfBook();
        std::cout << "[BOOK] orders=" << book.orderCount();
        if (tob.has_bid) {
            std::cout << " best_bid=" << tob.best_bid_price << '@' << tob.best_bid_qty;
        }
        if (tob.has_ask) {
            std::cout << " best_ask=" << tob.best_ask_price << '@' << tob.best_ask_qty;
        }
        std::cout << '\n';
    });

    std::array<epoll_event, kEpollMaxEvents> events{};
    std::array<std::uint8_t, kRecvBufferSize> recvBuffer{};
    std::vector<Socket> clients;

    auto closeClient = [&](int fd) {
        removeFdFromEpoll(epfd, fd);
        auto it = std::find_if(clients.begin(), clients.end(), [&](const Socket& s) { return s.fd() == fd; });
        if (it != clients.end()) {
            it->close();
            clients.erase(it);
        }
    };

    while (g_running.load(std::memory_order_relaxed)) {
        const int n = ::epoll_wait(epfd, events.data(), static_cast<int>(events.size()), 1000);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << "epoll_wait failed\n";
            break;
        }

        for (int i = 0; i < n; ++i) {
            const int fd = events[i].data.fd;
            const std::uint32_t ev = events[i].events;

            if ((ev & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) != 0U && fd != ingress.fd()) {
                closeClient(fd);
                continue;
            }

            if (fd == ingress.fd() && cfg.type == SocketType::TCP) {
                while (true) {
                    Socket client = ingress.accept();
                    if (client.fd() < 0) {
                        if (errno == EINTR) {
                            continue;
                        }
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        break;
                    }

                    if (!addFdToEpoll(epfd, client.fd(), EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                        client.close();
                        continue;
                    }

                    clients.emplace_back(std::move(client));
                }
                continue;
            }

            while (true) {
                const ssize_t r = ::recv(fd, recvBuffer.data(), recvBuffer.size(), 0);
                if (r > 0) {
                    std::size_t offset = 0;
                    std::size_t remaining = static_cast<std::size_t>(r);

                    while (remaining > 0) {
                        FeedChunk chunk{};
                        const std::size_t chunkLen = std::min(kChunkPayloadSize, remaining);
                        chunk.len = static_cast<std::uint16_t>(chunkLen);
                        std::memcpy(chunk.data.data(), recvBuffer.data() + offset, chunkLen);

                        while (!ringPtr->tryPush(chunk)) {
                            ++chunkBackpressure;
                            if (!g_running.load(std::memory_order_relaxed)) {
                                break;
                            }
                            std::this_thread::yield();
                        }

                        offset += chunkLen;
                        remaining -= chunkLen;
                    }
                    continue;
                }

                if (r == 0) {
                    if (fd != ingress.fd()) {
                        closeClient(fd);
                    }
                    break;
                }

                if (errno == EINTR) {
                    continue;
                }
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }

                if (fd != ingress.fd()) {
                    closeClient(fd);
                }
                break;
            }
        }
    }

    g_running.store(false, std::memory_order_relaxed);

    for (auto& c : clients) {
        c.close();
    }

    const int ingressFd = ingress.fd();
    removeFdFromEpoll(epfd, ingressFd);
    ingress.close();
    ::close(epfd);

    parserThread.join();

    std::cout << "[INGEST] chunk_backpressure_events=" << chunkBackpressure.load() << '\n';
    return 0;
}
