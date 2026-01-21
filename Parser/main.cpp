#include <iostream>
#include <vector>
#include "ITCH.h"

using u8 = uint8_t;

int main() {
    ITCH parser;

    // ----- Construct a fake but well-formed 'A' (Add Order) message -----
    // Layout (from your table):
    // 0  : MsgType ('A')
    // 1-2: Stock Locate (2 bytes)
    // 3-4: Tracking No (2 bytes)
    // 5-10: Timestamp (6 bytes)
    // 11-18: Order Ref (8 bytes)
    // 19: Side ('B' or 'S')
    // 20-23: Shares (4 bytes)
    // 24-31: Stock (8 bytes, space padded)
    // 32-35: Price (4 bytes, scaled by 100)

    std::vector<u8> buf(36, 0);

    // Message type
    buf[0] = 'A';

    // Stock Locate = 123 -> 00 7B
    buf[1] = 0x00;
    buf[2] = 0x7B;

    // Tracking No = 5555 -> 15 B3
    buf[3] = 0x15;
    buf[4] = 0xB3;

    // Timestamp = 1,000,000 (just an example, 6 bytes big-endian)
    uint64_t ts = 1'000'000;
    for (int i = 0; i < 6; i++) {
        buf[10 - i] = ts & 0xFF;   // fill backwards (big-endian)
        ts >>= 8;
    }

    // Order Ref = 90000001 (8 bytes)
    uint64_t ord = 90000001;
    for (int i = 0; i < 8; i++) {
        buf[18 - i] = ord & 0xFF;
        ord >>= 8;
    }

    // Side = 'B'
    buf[19] = 'B';

    // Shares = 1000 -> 00 00 03 E8
    buf[20] = 0x00;
    buf[21] = 0x00;
    buf[22] = 0x03;
    buf[23] = 0xE8;

    // Stock = "AAPL    " (8 bytes)
    std::string sym = "AAPL    ";
    for (int i = 0; i < 8; i++) {
        buf[24 + i] = sym[i];
    }

    // Price = 150.25 -> 15025 decimal -> 00 00 3A C1
    uint32_t price = 15025;
    buf[32] = (price >> 24) & 0xFF;
    buf[33] = (price >> 16) & 0xFF;
    buf[34] = (price >> 8) & 0xFF;
    buf[35] = price & 0xFF;

    std::vector<u8> bigbuf;
    bigbuf.insert(bigbuf.end(), buf.begin(), buf.end());
    bigbuf.insert(bigbuf.end(), buf.begin(), buf.end()); // same message twice

    // ---- Feed to parser ----
    parser.processBuffer(bigbuf.data(), bigbuf.size());

    // ---- Manually call parseMsg to inspect output ----
    auto fields = parser.parseMsg(buf);

    std::cout << "Parsed fields:\n";
    for (size_t i = 0; i < fields.size(); i++) {
        // std::cout << i << " : [" << fields[i].type << "]\n";
        if (fields[i].is_string)
            std::cout << " = \"" << fields[i].s << "\"\n";
        else
            std::cout << " = " << fields[i].u64 << "\n";
    }

    return 0;
}
