#ifndef APEXALPHA_MODELS_TRADE_H
#define APEXALPHA_MODELS_TRADE_H

#include <cstdint>
#include <string>

namespace apex::alpha {

enum class TradeSide : std::uint8_t {
    BUY,
    SELL
};

struct Trade {
    std::uint64_t timestamp{0};
    std::string symbol{};
    TradeSide side{TradeSide::BUY};
    std::uint32_t size{0};
    double price{0.0};
};

inline const char* toString(TradeSide side) {
    return side == TradeSide::BUY ? "BUY" : "SELL";
}

}  // namespace apex::alpha

#endif
