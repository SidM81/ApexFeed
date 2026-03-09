#ifndef SOCKET_TYPES_H
#define SOCKET_TYPES_H

enum class SocketType {
    TCP,
    UDP,
    UDP_MULTICAST
};

enum class SocketRole {
    CLIENT,
    SERVER
};

#endif
