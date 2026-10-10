#ifndef APEXALPHA_STRATEGY_STRATEGY_CONFIG_H
#define APEXALPHA_STRATEGY_STRATEGY_CONFIG_H

#include <cstdint>
#include <string>

namespace apex::alpha::strategy {

struct StrategyConfig {
    std::uint32_t order_size{100};
    std::int64_t max_position{1000};
    double buy_qi_threshold{0.20};
    double sell_qi_threshold{-0.20};
    double buy_ofi_threshold{0.0};
    double sell_ofi_threshold{0.0};
    double exit_long_qi_threshold{-0.05};
    double exit_short_qi_threshold{0.05};
    double exit_long_ofi_threshold{-1.0};
    double exit_short_ofi_threshold{1.0};

    static StrategyConfig loadFromFile(const std::string& path);
};

}  // namespace apex::alpha::strategy

#endif
