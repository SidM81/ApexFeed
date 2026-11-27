#ifndef APEXALPHA_FEATURE_FEATURE_STATE_H
#define APEXALPHA_FEATURE_FEATURE_STATE_H

#include <cstdint>

namespace apex::alpha::feature {

struct FeatureState {
    std::uint64_t previous_bid_volume{0};
    std::uint64_t previous_ask_volume{0};
    bool has_previous{false};
};

}  // namespace apex::alpha::feature

#endif
