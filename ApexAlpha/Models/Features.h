#ifndef APEXALPHA_MODELS_FEATURES_H
#define APEXALPHA_MODELS_FEATURES_H

#include <cstdint>
#include <string>

namespace apex::alpha {

struct FeatureSnapshot {
    std::string symbol{};
    std::uint64_t timestamp{0};
    double spread{0.0};
    double mid{0.0};
    double microprice{0.0};
    double queue_imbalance{0.0};
    double order_flow_imbalance{0.0};
    bool has_valid_tob{false};
};

}  // namespace apex::alpha

#endif