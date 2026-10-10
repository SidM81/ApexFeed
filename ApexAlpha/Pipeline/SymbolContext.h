#ifndef APEXALPHA_PIPELINE_SYMBOL_CONTEXT_H
#define APEXALPHA_PIPELINE_SYMBOL_CONTEXT_H

#include <cstdint>
#include <memory>
#include <string>

#include "ApexAlpha/Feature/FeatureEngine.h"
#include "ApexAlpha/Simulation/ExecutionSimulator.h"
#include "ApexAlpha/Simulation/PnlTracker.h"
#include "ApexAlpha/Strategy/IStrategy.h"
#include "ApexAlpha/Strategy/StrategyConfig.h"
#include "ApexAlpha/Strategy/ThresholdStrategy.h"
#include "OrderBook/OrderBook.h"

namespace apex::alpha::pipeline {

struct SymbolContext {
    explicit SymbolContext(std::string symbolName, const strategy::StrategyConfig& config)
        : symbol(std::move(symbolName)),
          feature_engine(symbol),
          strategy(std::make_unique<strategy::ThresholdStrategy>(config)),
          simulator(symbol, config.max_position),
          pnl(symbol) {}

    std::string symbol{};
    apex::OrderBook book{};
    feature::FeatureEngine feature_engine;
    std::unique_ptr<strategy::IStrategy> strategy{};
    simulation::ExecutionSimulator simulator;
    simulation::PnlTracker pnl;
    std::uint64_t processed_messages{0};
    std::uint64_t trade_count{0};
};

}  // namespace apex::alpha::pipeline

#endif
