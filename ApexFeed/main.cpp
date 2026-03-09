#include <array>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <iostream>

#include <cerrno>

#include "Core/MessageStats.h"
#include "OrderBook/OrderBook.h"
#include "Parser/ITCH.h"
#include "Socket/Socket.h"

namespace {
char messageType(const Field* fields, std::size_t count) {
    if (count == 0 || fields[0].type != TYPE::MSGTYPE || fields[0].ascii_len == 0) {
        return '?';
    }
    return fields[0].ascii[0];
}
}  // namespace

int main() {
    ITCH parser;
    apex::OrderBook book;
    apex::MessageStats stats(100000);

    SocketConfig cfg{};
    cfg.type = SocketType::TCP;
    cfg.role = SocketRole::SERVER;
    cfg.ip = "127.0.0.1";
    cfg.port = 9001;
    cfg.non_blocking = false;
    cfg.reuse_addr = true;

    Socket server(cfg);
    if (!server.open()) {
        std::cerr << "failed to open server socket\n";
        return 1;
    }

    Socket client = server.accept();
    if (client.fd() < 0) {
        std::cerr << "accept failed\n";
        return 1;
    }

    std::array<std::uint8_t, 65536> recvBuffer{};

    while (true) {
        const ssize_t n = client.recv(recvBuffer.data(), recvBuffer.size());
        if (n > 0) {
            std::array<std::uint64_t, 256> localCounts{};
            std::uint64_t parsed = 0;
            const auto parseStart = std::chrono::steady_clock::now();

            parser.processBuffer(recvBuffer.data(), static_cast<std::size_t>(n), [&](const Field* fields, std::size_t count) {
                const char type = messageType(fields, count);
                ++localCounts[static_cast<unsigned char>(type)];
                ++parsed;
                book.applyITCH(fields, count);
            });

            const auto parseEnd = std::chrono::steady_clock::now();
            const auto parseNs = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(parseEnd - parseStart).count());
            stats.onBatch(localCounts, parsed, parseNs);
            continue;
        }

        if (n == 0) {
            break;
        }

        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;
        }

        std::perror("recv");
        break;
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

    return 0;
}

