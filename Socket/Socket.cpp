#include "Socket.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

Socket::Socket(const SocketConfig& cfg)
    : config(cfg) {}

Socket::Socket(int fd)
    : sockfd(fd) {}

Socket::~Socket() {
    close();
}

Socket Socket::accept() {
    int clientFd = ::accept(sockfd, nullptr, nullptr);
    if (clientFd < 0) {
        return Socket(-1);
    }
    return Socket(clientFd);
}

bool Socket::open() {
    if (config.type == SocketType::TCP)
        return setupTCP();
    else
        return setupUDP();
}

bool Socket::setupTCP() {
    sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return false;

    if (config.reuse_addr) setReuseAddr();
    if (config.non_blocking) makeNonBlocking();

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(config.port);
    inet_pton(AF_INET, config.ip.c_str(), &addr.sin_addr);

    if (config.role == SocketRole::CLIENT) {
        return ::connect(sockfd,
                         reinterpret_cast<sockaddr*>(&addr),
                         sizeof(addr)) == 0
               || errno == EINPROGRESS;
    } else {
        if (::bind(sockfd,
                   reinterpret_cast<sockaddr*>(&addr),
                   sizeof(addr)) < 0)
            return false;

        return ::listen(sockfd, 128) == 0;
    }
}

bool Socket::setupUDP() {
    sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return false;

    if (config.reuse_addr) setReuseAddr();
    if (config.non_blocking) makeNonBlocking();

    // bind/connect logic depends on role
    return true;
}

bool Socket::makeNonBlocking() {
    int flags = fcntl(sockfd, F_GETFL, 0);
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == 0;
}

bool Socket::setReuseAddr() {
    int opt = 1;
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                      &opt, sizeof(opt)) == 0;
}

ssize_t Socket::recv(uint8_t* buffer, size_t len) {
    return ::recv(sockfd, buffer, len, 0);
}

ssize_t Socket::send(const uint8_t* data, size_t len) {
    return ::send(sockfd, data, len, 0);
}

bool Socket::close() {
    if (sockfd >= 0) {
        ::close(sockfd);
        sockfd = -1;
    }
    return true;
}
