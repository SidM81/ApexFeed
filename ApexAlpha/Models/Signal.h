#ifndef APEXALPHA_MODELS_SIGNAL_H
#define APEXALPHA_MODELS_SIGNAL_H

#include <cstdint>

namespace apex::alpha {

enum class SignalSide : std::uint8_t {
    BUY,
    SELL,
    HOLD,
    EXIT
};

struct SignalDecision {
    SignalSide side{SignalSide::HOLD};
    std::uint32_t size{0};
};

inline const char* toString(SignalSide side) {
    switch (side) {
        case SignalSide::BUY:
            return "BUY";
        case SignalSide::SELL:
            return "SELL";
        case SignalSide::EXIT:
            return "EXIT";
        case SignalSide::HOLD:
        default:
            return "HOLD";
    }
}

}  // namespace apex::alpha

#endif
