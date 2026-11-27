#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "ApexAlpha/Pipeline/AlphaEngine.h"
#include "ApexAlpha/Strategy/StrategyConfig.h"
#include "Parser/ITCH.h"

namespace {

struct ReplayConfig {
    std::string replay_file{};
    std::size_t chunk_size{65536};
    std::uint64_t report_every{100000};
    std::string out_csv{"Benchmarks/results/apexalpha_events.csv"};
    std::string strategy_config{};
};

ReplayConfig parseConfig(int argc, char** argv) {
    ReplayConfig cfg{};
    if (argc >= 2) {
        cfg.replay_file = argv[1];
    }

    for (int i = 2; i < argc; ++i) {
        const std::string arg(argv[i]);
        if (arg.rfind("--chunk=", 0) == 0) {
            cfg.chunk_size = static_cast<std::size_t>(std::stoull(arg.substr(8)));
        } else if (arg.rfind("--report=", 0) == 0) {
            cfg.report_every = static_cast<std::uint64_t>(std::stoull(arg.substr(9)));
        } else if (arg.rfind("--out=", 0) == 0) {
            cfg.out_csv = arg.substr(6);
        } else if (arg.rfind("--strategy-config=", 0) == 0) {
            cfg.strategy_config = arg.substr(18);
        }
    }

    return cfg;
}

void printProgress(std::uint64_t parsed, const std::chrono::steady_clock::time_point& start) {
    const auto now = std::chrono::steady_clock::now();
    const double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(now - start).count();
    const double msgPerSecond = seconds > 0.0 ? static_cast<double>(parsed) / seconds : 0.0;

    std::cout << "[ALPHA][PROGRESS] parsed=" << parsed
              << " elapsed=" << seconds
              << " msg/sec=" << msgPerSecond
              << '\n';
}

}  // namespace

int main(int argc, char** argv) {
    const ReplayConfig cfg = parseConfig(argc, argv);
    if (cfg.replay_file.empty()) {
        std::cerr << "usage: replay_strategy <binary_itch_file> [--chunk=65536] [--report=100000] "
                  << "[--out=Benchmarks/results/apexalpha_events.csv] [--strategy-config=path]\n";
        return 1;
    }

    std::ifstream input(cfg.replay_file, std::ios::binary);
    if (!input) {
        std::cerr << "failed to open replay file: " << cfg.replay_file << '\n';
        return 1;
    }

    const auto strategyConfig = apex::alpha::strategy::StrategyConfig::loadFromFile(cfg.strategy_config);
    apex::alpha::pipeline::AlphaEngine engine(strategyConfig, cfg.out_csv);

    ITCH parser;
    std::vector<std::uint8_t> buffer(cfg.chunk_size);

    std::uint64_t parsedMessages = 0;
    const auto start = std::chrono::steady_clock::now();

    while (input) {
        input.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        const std::streamsize readBytes = input.gcount();
        if (readBytes <= 0) {
            break;
        }

        parser.processBuffer(buffer.data(), static_cast<std::size_t>(readBytes), [&](const Field* fields, std::size_t count) {
            ++parsedMessages;
            engine.onMessage(fields, count);

            if (cfg.report_every > 0 && parsedMessages % cfg.report_every == 0) {
                printProgress(parsedMessages, start);
            }
        });
    }

    engine.finalize(std::cout);
    printProgress(parsedMessages, start);
    std::cout << "[ALPHA] csv_output=" << cfg.out_csv << '\n';
    return 0;
}
