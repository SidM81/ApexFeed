#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Core/MessageStats.h"
#include "Parser/ITCH.h"

namespace {
char messageType(const Field* fields, std::size_t count) {
    if (count == 0 || fields[0].type != TYPE::MSGTYPE || fields[0].ascii_len == 0) {
        return '?';
    }
    return fields[0].ascii[0];
}
}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: replay_feed <binary_itch_file> [--chunk=65536] [--report=100000]\n";
        return 1;
    }

    std::size_t chunkSize = 65536;
    std::uint64_t reportEvery = 100000;

    for (int i = 2; i < argc; ++i) {
        const std::string arg(argv[i]);
        if (arg.rfind("--chunk=", 0) == 0) {
            chunkSize = static_cast<std::size_t>(std::stoull(arg.substr(8)));
        } else if (arg.rfind("--report=", 0) == 0) {
            reportEvery = static_cast<std::uint64_t>(std::stoull(arg.substr(9)));
        }
    }

    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        std::cerr << "failed to open replay file: " << argv[1] << '\n';
        return 1;
    }

    ITCH parser;
    apex::MessageStats stats(reportEvery);
    std::vector<std::uint8_t> buffer(chunkSize);

    while (input) {
        input.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        const std::streamsize readBytes = input.gcount();
        if (readBytes <= 0) {
            break;
        }

        std::array<std::uint64_t, 256> localCounts{};
        std::uint64_t localMessages = 0;
        const auto parseStart = std::chrono::steady_clock::now();

        parser.processBuffer(buffer.data(), static_cast<std::size_t>(readBytes), [&](const Field* fields, std::size_t count) {
            const char type = messageType(fields, count);
            ++localCounts[static_cast<unsigned char>(type)];
            ++localMessages;
        });

        const auto parseEnd = std::chrono::steady_clock::now();
        const auto parseNs = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(parseEnd - parseStart).count());

        stats.onBatch(localCounts, localMessages, parseNs);
    }

    stats.report(true);
    stats.printTypeBreakdown();
    return 0;
}

