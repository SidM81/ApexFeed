#pragma once

#include <string>
#include <cstdint>
#include "SocketTypes.h"

struct SocketConfig {
    SocketType type;
    SocketRole role;

    std::string ip;   // "127.0.0.1" or multicast group
    uint16_t port;

    bool non_blocking = true;
    bool reuse_addr   = true;
};


class Socket {
public:
    explicit Socket(const SocketConfig& cfg);
    explicit Socket(int fd); 
    ~Socket();

    bool open();
    bool close();
    
    Socket accept();

    ssize_t send(const uint8_t* data, size_t len);
    ssize_t recv(uint8_t* buffer, size_t len);

    int fd() const { return sockfd; }

private:
    bool setupTCP();
    bool setupUDP();
    bool makeNonBlocking();
    bool setReuseAddr();

private:
    int sockfd{-1};
    SocketConfig config;
};
