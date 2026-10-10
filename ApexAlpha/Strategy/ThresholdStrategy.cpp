#include "ApexAlpha/Strategy/ThresholdStrategy.h"

#include <cstdlib>

namespace apex::alpha::strategy {

ThresholdStrategy::ThresholdStrategy(StrategyConfig config)
    : config_(config) {}

SignalDecision ThresholdStrategy::onFeatures(const FeatureSnapshot& features,
                                             const PortfolioState& portfolio) {
    SignalDecision decision{};

    if (!features.has_valid_tob) {
        decision.side = SignalSide::HOLD;
        return decision;
    }

    const bool exitLong = portfolio.position > 0 &&
        (features.queue_imbalance <= config_.exit_long_qi_threshold ||
         features.order_flow_imbalance <= config_.exit_long_ofi_threshold);

    if (exitLong) {
        decision.side = SignalSide::EXIT;
        decision.size = static_cast<std::uint32_t>(std::llabs(portfolio.position));
        return decision;
    }

    const bool exitShort = portfolio.position < 0 &&
        (features.queue_imbalance >= config_.exit_short_qi_threshold ||
         features.order_flow_imbalance >= config_.exit_short_ofi_threshold);

    if (exitShort) {
        decision.side = SignalSide::EXIT;
        decision.size = static_cast<std::uint32_t>(std::llabs(portfolio.position));
        return decision;
    }

    if (portfolio.position != 0) {
        decision.side = SignalSide::HOLD;
        return decision;
    }

    const bool enterLong =
        features.queue_imbalance >= config_.buy_qi_threshold &&
        features.order_flow_imbalance >= config_.buy_ofi_threshold;

    const bool enterShort =
        features.queue_imbalance <= config_.sell_qi_threshold &&
        features.order_flow_imbalance <= config_.sell_ofi_threshold;

    if (enterLong && portfolio.position + static_cast<std::int64_t>(config_.order_size) <= config_.max_position) {
        decision.side = SignalSide::BUY;
        decision.size = config_.order_size;
        return decision;
    }

    if (enterShort && portfolio.position - static_cast<std::int64_t>(config_.order_size) >= -config_.max_position) {
        decision.side = SignalSide::SELL;
        decision.size = config_.order_size;
        return decision;
    }

    decision.side = SignalSide::HOLD;
    return decision;
}

}  // namespace apex::alpha::strategy
