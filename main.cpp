#include "Parser/ITCH.h"
#include "Socket/Socket.h"
#include <array>

int main() {
    ITCH parser;

    SocketConfig cfg{
        .type = SocketType::UDP,
        .role = SocketRole::CLIENT,
        .ip = "239.1.1.1",
        .port = 5000,
        .non_blocking = true,
        .reuse_addr = true
    };

    Socket sock(cfg);
    sock.open();

    std::array<uint8_t, 4096> buffer;

    while (true) {
        ssize_t n = sock.recv(buffer.data(), buffer.size());
        if (n > 0) {
            parser.processBuffer(buffer.data(), n);
        }
    }
}
