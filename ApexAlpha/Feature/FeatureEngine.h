#ifndef APEXALPHA_FEATURE_FEATURE_ENGINE_H
#define APEXALPHA_FEATURE_FEATURE_ENGINE_H

#include <cstdint>
#include <string>

#include "ApexAlpha/Feature/FeatureState.h"
#include "ApexAlpha/Models/Features.h"
#include "OrderBook/OrderBook.h"

namespace apex::alpha::feature {

class FeatureEngine {
public:
    explicit FeatureEngine(std::string symbol);

    FeatureSnapshot update(const apex::OrderBook::TopOfBook& topOfBook, std::uint64_t timestamp);

private:
    std::string symbol_{};
    FeatureState state_{};
};

}  // namespace apex::alpha::feature

#endif
