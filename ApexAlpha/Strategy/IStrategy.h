#ifndef APEXALPHA_STRATEGY_ISTRATEGY_H
#define APEXALPHA_STRATEGY_ISTRATEGY_H

#include "ApexAlpha/Models/Features.h"
#include "ApexAlpha/Models/PortfolioState.h"
#include "ApexAlpha/Models/Signal.h"

namespace apex::alpha::strategy {

class IStrategy {
public:
    virtual ~IStrategy() = default;

    virtual SignalDecision onFeatures(const FeatureSnapshot& features,
                                      const PortfolioState& portfolio) = 0;
};

}  // namespace apex::alpha::strategy

#endif
