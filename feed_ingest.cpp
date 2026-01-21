#include<iostream>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<linux/if_packet.h>
#include<unistd.h>
#include<chrono>
#include<vector>
#include<cstring>

using namespace std;

static const int BATCH = 128;
static const int PKT_BUF_SZ = 2048;

int main() {
    int fd = socket(AF_INET,SOCK_DGRAM, 0);
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));


    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000); // replace with feed port
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(fd, (sockaddr*)&addr, sizeof(addr));

    vector<mmsghdr> msgs(BATCH);
    vector<iovec> iov(BATCH);
    vector<std::array<char,PKT_BUF_SZ>> bufs(BATCH);
}

// 1st Class Msg Decoder/Parser
// 2nd Class for socket setup direct call with info and you get a socket and connect, send and receive all wrapper of this class for easy handling
// 3rd Class ITCH protocol handler (1st class will be parent)
// 4th Class for FIX protocol handler (1st class will be parent)
// 5th Class Orderbook class (in-memory for now)
// 6th Ingester (TCP/UDP) (will use 2nd class as helper and save in 5th class)
// 7th Class perfomance Tester 