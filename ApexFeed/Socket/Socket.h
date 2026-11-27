#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <sys/types.h>
#include "SocketTypes.h"

struct SocketConfig {
    SocketType type{SocketType::TCP};
    SocketRole role{SocketRole::CLIENT};
    std::string ip{"127.0.0.1"};
    std::string multicast_interface{"0.0.0.0"};
    std::uint16_t port{0};
    bool non_blocking{true};
    bool reuse_addr{true};
    bool reuse_port{false};
    int backlog{128};
    int multicast_ttl{1};
};

class Socket {
public:
    explicit Socket(const SocketConfig& cfg);
    Socket(int fd, const SocketConfig& cfg);
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    bool open();
    bool close();

    [[nodiscard]] Socket accept() const;

    ssize_t send(const std::uint8_t* data, std::size_t len) const;
    ssize_t sendAll(const std::uint8_t* data, std::size_t len) const;
    ssize_t recv(std::uint8_t* buffer, std::size_t len) const;

    [[nodiscard]] int fd() const { return sockfd_; }
    [[nodiscard]] bool isOpen() const { return sockfd_ >= 0; }

private:
    bool setupTCP();
    bool setupUDP();
    bool setupMulticast();
    bool makeNonBlocking() const;
    bool setReuseAddr() const;
    bool setReusePort() const;

private:
    int sockfd_{-1};
    SocketConfig config_{};
};
