#ifndef APEXALPHA_STRATEGY_THRESHOLD_STRATEGY_H
#define APEXALPHA_STRATEGY_THRESHOLD_STRATEGY_H

#include "ApexAlpha/Strategy/IStrategy.h"
#include "ApexAlpha/Strategy/StrategyConfig.h"

namespace apex::alpha::strategy {

class ThresholdStrategy final : public IStrategy {
public:
    explicit ThresholdStrategy(StrategyConfig config);

    SignalDecision onFeatures(const FeatureSnapshot& features,
                              const PortfolioState& portfolio) override;

private:
    StrategyConfig config_{};
};

}  // namespace apex::alpha::strategy

#endif
