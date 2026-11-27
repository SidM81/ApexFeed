#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#if defined(__x86_64__) || defined(__i386__)
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#endif

#include "Parser/ITCH.h"

namespace {

std::array<std::uint8_t, 36> sampleAddOrder() {
    std::array<std::uint8_t, 36> msg{};
    msg[0] = 'A';
    msg[1] = 0x00;
    msg[2] = 0x7B;
    msg[3] = 0x15;
    msg[4] = 0xB3;

    std::uint64_t ts = 1'000'000;
    for (int i = 0; i < 6; ++i) {
        msg[10 - i] = static_cast<std::uint8_t>(ts & 0xFFU);
        ts >>= 8;
    }

    std::uint64_t ord = 90000001;
    for (int i = 0; i < 8; ++i) {
        msg[18 - i] = static_cast<std::uint8_t>(ord & 0xFFU);
        ord >>= 8;
    }

    msg[19] = 'B';
    msg[20] = 0;
    msg[21] = 0;
    msg[22] = 3;
    msg[23] = 0xE8;

    std::memcpy(&msg[24], "AAPL    ", 8);

    const std::uint32_t price = 15025;
    msg[32] = static_cast<std::uint8_t>((price >> 24) & 0xFFU);
    msg[33] = static_cast<std::uint8_t>((price >> 16) & 0xFFU);
    msg[34] = static_cast<std::uint8_t>((price >> 8) & 0xFFU);
    msg[35] = static_cast<std::uint8_t>(price & 0xFFU);

    return msg;
}

}  // namespace

int main(int argc, char** argv) {
    std::uint64_t iterations = 5'000'000;
    if (argc > 1) {
        iterations = static_cast<std::uint64_t>(std::stoull(argv[1]));
    }

    const auto msg = sampleAddOrder();
    ITCH parser;

    Field fields[kMaxParsedFields];
    std::size_t fieldCount = 0;

    // warmup
    for (std::uint64_t i = 0; i < 10000; ++i) {
        parser.parseMsg(msg.data(), fields, fieldCount);
    }

    const auto wallStart = std::chrono::steady_clock::now();

#if defined(__x86_64__) || defined(__i386__)
    const std::uint64_t cycleStart = __rdtsc();
#endif

    for (std::uint64_t i = 0; i < iterations; ++i) {
        parser.parseMsg(msg.data(), fields, fieldCount);
    }

#if defined(__x86_64__) || defined(__i386__)
    const std::uint64_t cycleEnd = __rdtsc();
#endif

    const auto wallEnd = std::chrono::steady_clock::now();
    const auto totalNs = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(wallEnd - wallStart).count());

    const double nsPerMsg = iterations > 0 ? static_cast<double>(totalNs) / static_cast<double>(iterations) : 0.0;

#if defined(__x86_64__) || defined(__i386__)
    const std::uint64_t totalCycles = cycleEnd - cycleStart;
    const double cyclesPerMsg = iterations > 0 ? static_cast<double>(totalCycles) / static_cast<double>(iterations) : 0.0;
    std::cout << "[LAT] iterations=" << iterations
              << " cycles_per_msg=" << cyclesPerMsg
              << " ns_per_msg=" << nsPerMsg
              << '\n';
#else
    std::cout << "[LAT] iterations=" << iterations
              << " ns_per_msg=" << nsPerMsg
              << " (rdtsc unavailable on this architecture)"
              << '\n';
#endif

    return 0;
}
