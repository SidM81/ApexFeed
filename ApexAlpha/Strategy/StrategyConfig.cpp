#include "ApexAlpha/Strategy/StrategyConfig.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>

namespace apex::alpha::strategy {

namespace {

std::string trim(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char c) {
        return !std::isspace(c);
    }));

    value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char c) {
        return !std::isspace(c);
    }).base(), value.end());

    return value;
}

void assignDouble(const std::string& key, const std::string& value, StrategyConfig& cfg) {
    const double v = std::stod(value);
    if (key == "buy_qi_threshold") {
        cfg.buy_qi_threshold = v;
    } else if (key == "sell_qi_threshold") {
        cfg.sell_qi_threshold = v;
    } else if (key == "buy_ofi_threshold") {
        cfg.buy_ofi_threshold = v;
    } else if (key == "sell_ofi_threshold") {
        cfg.sell_ofi_threshold = v;
    } else if (key == "exit_long_qi_threshold") {
        cfg.exit_long_qi_threshold = v;
    } else if (key == "exit_short_qi_threshold") {
        cfg.exit_short_qi_threshold = v;
    } else if (key == "exit_long_ofi_threshold") {
        cfg.exit_long_ofi_threshold = v;
    } else if (key == "exit_short_ofi_threshold") {
        cfg.exit_short_ofi_threshold = v;
    }
}

}  // namespace

StrategyConfig StrategyConfig::loadFromFile(const std::string& path) {
    StrategyConfig cfg{};

    if (path.empty()) {
        return cfg;
    }

    std::ifstream input(path);
    if (!input) {
        std::cerr << "[ApexAlpha] failed to open strategy config: " << path << '\n';
        return cfg;
    }

    std::string line;
    while (std::getline(input, line)) {
        const std::size_t comment = line.find('#');
        if (comment != std::string::npos) {
            line = line.substr(0, comment);
        }

        const std::size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        const std::string key = trim(line.substr(0, eq));
        const std::string value = trim(line.substr(eq + 1));
        if (key.empty() || value.empty()) {
            continue;
        }

        try {
            if (key == "order_size") {
                cfg.order_size = static_cast<std::uint32_t>(std::stoul(value));
            } else if (key == "max_position") {
                cfg.max_position = static_cast<std::int64_t>(std::stoll(value));
            } else {
                assignDouble(key, value, cfg);
            }
        } catch (const std::exception&) {
            std::cerr << "[ApexAlpha] invalid config value for " << key << " in " << path << '\n';
        }
    }

    return cfg;
}

}  // namespace apex::alpha::strategy
