#include "Socket.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

namespace {

inline bool isWouldBlock(int e) {
    return e == EAGAIN || e == EWOULDBLOCK;
}

}  // namespace

Socket::Socket(const SocketConfig& cfg)
    : config_(cfg) {}

Socket::Socket(int fd, const SocketConfig& cfg)
    : sockfd_(fd), config_(cfg) {}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) noexcept
    : sockfd_(other.sockfd_), config_(other.config_) {
    other.sockfd_ = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    close();
    sockfd_ = other.sockfd_;
    config_ = other.config_;
    other.sockfd_ = -1;
    return *this;
}

bool Socket::open() {
    if (sockfd_ >= 0) {
        return true;
    }

    switch (config_.type) {
        case SocketType::TCP:
            return setupTCP();
        case SocketType::UDP:
            return setupUDP();
        case SocketType::UDP_MULTICAST:
            return setupMulticast();
        default:
            return false;
    }
}

bool Socket::setupTCP() {
    sockfd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        return false;
    }

    if (config_.reuse_addr && !setReuseAddr()) {
        close();
        return false;
    }
    if (config_.reuse_port && !setReusePort()) {
        close();
        return false;
    }
    if (config_.non_blocking && !makeNonBlocking()) {
        close();
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(config_.port);

    if (::inet_pton(AF_INET, config_.ip.c_str(), &addr.sin_addr) != 1) {
        close();
        return false;
    }

    if (config_.role == SocketRole::CLIENT) {
        if (::connect(sockfd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0) {
            return true;
        }
        if (errno == EINPROGRESS) {
            return true;
        }
        close();
        return false;
    }

    if (::bind(sockfd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close();
        return false;
    }

    if (::listen(sockfd_, config_.backlog) < 0) {
        close();
        return false;
    }

    return true;
}

bool Socket::setupUDP() {
    sockfd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        return false;
    }

    if (config_.reuse_addr && !setReuseAddr()) {
        close();
        return false;
    }
    if (config_.reuse_port && !setReusePort()) {
        close();
        return false;
    }
    if (config_.non_blocking && !makeNonBlocking()) {
        close();
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(config_.port);
    if (::inet_pton(AF_INET, config_.ip.c_str(), &addr.sin_addr) != 1) {
        close();
        return false;
    }

    if (config_.role == SocketRole::SERVER) {
        if (::bind(sockfd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            close();
            return false;
        }
        return true;
    }

    if (::connect(sockfd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0 && errno != EINPROGRESS) {
        close();
        return false;
    }

    return true;
}

bool Socket::setupMulticast() {
    sockfd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        return false;
    }

    if (config_.reuse_addr && !setReuseAddr()) {
        close();
        return false;
    }
    if (config_.reuse_port && !setReusePort()) {
        close();
        return false;
    }
    if (config_.non_blocking && !makeNonBlocking()) {
        close();
        return false;
    }

    if (config_.role == SocketRole::SERVER) {
        sockaddr_in bindAddr{};
        bindAddr.sin_family = AF_INET;
        bindAddr.sin_port = htons(config_.port);
        bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (::bind(sockfd_, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) < 0) {
            close();
            return false;
        }

        ip_mreq mreq{};
        if (::inet_pton(AF_INET, config_.ip.c_str(), &mreq.imr_multiaddr) != 1) {
            close();
            return false;
        }

        if (::inet_pton(AF_INET, config_.multicast_interface.c_str(), &mreq.imr_interface) != 1) {
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        }

        if (::setsockopt(sockfd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
            close();
            return false;
        }
        return true;
    }

    unsigned char ttl = static_cast<unsigned char>(config_.multicast_ttl);
    if (::setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        close();
        return false;
    }

    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port = htons(config_.port);
    if (::inet_pton(AF_INET, config_.ip.c_str(), &target.sin_addr) != 1) {
        close();
        return false;
    }

    if (::connect(sockfd_, reinterpret_cast<sockaddr*>(&target), sizeof(target)) < 0 && errno != EINPROGRESS) {
        close();
        return false;
    }

    return true;
}

bool Socket::makeNonBlocking() const {
    const int flags = ::fcntl(sockfd_, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return ::fcntl(sockfd_, F_SETFL, flags | O_NONBLOCK) == 0;
}

bool Socket::setReuseAddr() const {
    int opt = 1;
    return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
}

bool Socket::setReusePort() const {
#ifdef SO_REUSEPORT
    int opt = 1;
    return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == 0;
#else
    return true;
#endif
}

Socket Socket::accept() const {
    if (sockfd_ < 0) {
        return Socket(-1, config_);
    }

    while (true) {
        const int clientFd = ::accept(sockfd_, nullptr, nullptr);
        if (clientFd >= 0) {
            if (config_.non_blocking) {
                const int flags = ::fcntl(clientFd, F_GETFL, 0);
                if (flags < 0 || ::fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) != 0) {
                    ::close(clientFd);
                    return Socket(-1, config_);
                }
            }

            SocketConfig acceptedCfg = config_;
            acceptedCfg.role = SocketRole::CLIENT;
            return Socket(clientFd, acceptedCfg);
        }

        if (errno == EINTR) {
            continue;
        }
        return Socket(-1, config_);
    }
}

ssize_t Socket::recv(std::uint8_t* buffer, std::size_t len) const {
    if (sockfd_ < 0 || buffer == nullptr || len == 0) {
        errno = EINVAL;
        return -1;
    }

    while (true) {
        const ssize_t n = ::recv(sockfd_, buffer, len, 0);
        if (n >= 0) {
            return n;
        }

        if (errno == EINTR) {
            continue;
        }

        if (isWouldBlock(errno)) {
            return -1;
        }

        return -1;
    }
}

ssize_t Socket::send(const std::uint8_t* data, std::size_t len) const {
    if (sockfd_ < 0 || data == nullptr || len == 0) {
        errno = EINVAL;
        return -1;
    }

    while (true) {
        const ssize_t n = ::send(sockfd_, data, len, MSG_NOSIGNAL);
        if (n >= 0) {
            return n;
        }

        if (errno == EINTR) {
            continue;
        }

        if (isWouldBlock(errno)) {
            return -1;
        }

        return -1;
    }
}

ssize_t Socket::sendAll(const std::uint8_t* data, std::size_t len) const {
    std::size_t sent = 0;
    while (sent < len) {
        const ssize_t n = send(data + sent, len - sent);
        if (n <= 0) {
            return sent > 0 ? static_cast<ssize_t>(sent) : n;
        }
        sent += static_cast<std::size_t>(n);
    }
    return static_cast<ssize_t>(sent);
}

bool Socket::close() {
    if (sockfd_ >= 0) {
        ::close(sockfd_);
        sockfd_ = -1;
    }
    return true;
}
