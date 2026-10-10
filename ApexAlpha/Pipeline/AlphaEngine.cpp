#include "ApexAlpha/Pipeline/AlphaEngine.h"

#include "ApexAlpha/IO/RunSummary.h"

namespace apex::alpha::pipeline {

AlphaEngine::AlphaEngine(strategy::StrategyConfig config, std::string outputPath)
    : config_(config),
      writer_(std::move(outputPath)) {
    contexts_.reserve(128);
}

SymbolContext& AlphaEngine::getOrCreateContext(const std::string& symbol) {
    auto found = contexts_.find(symbol);
    if (found != contexts_.end()) {
        return found->second;
    }

    auto [it, inserted] = contexts_.try_emplace(symbol, SymbolContext(symbol, config_));
    (void)inserted;
    return it->second;
}

bool AlphaEngine::onMessage(const Field* fields, std::size_t count) {
    ++stats_.total_messages;

    const RoutedMessage routed = router_.route(fields, count);
    if (!routed.book_event) {
        return false;
    }

    if (!routed.routed || routed.symbol.empty()) {
        ++stats_.unrouted_messages;
        return false;
    }

    ++stats_.routed_messages;

    SymbolContext& context = getOrCreateContext(routed.symbol);
    context.book.applyITCH(fields, count);

    const auto topOfBook = context.book.topOfBook();
    FeatureSnapshot features = context.feature_engine.update(topOfBook, routed.timestamp);
    ++stats_.feature_updates;

    const PortfolioState portfolioBefore = context.pnl.snapshot();
    const SignalDecision decision = context.strategy->onFeatures(features, portfolioBefore);
    if (decision.side != SignalSide::HOLD) {
        ++stats_.signal_events;
    }

    const auto trade = context.simulator.apply(decision, topOfBook, routed.timestamp);
    if (trade.has_value()) {
        context.pnl.onTrade(*trade);
        ++context.trade_count;
        ++stats_.trade_events;
    }

    context.pnl.markToMarket(features.mid);
    const PortfolioState portfolioAfter = context.pnl.snapshot();

    EventRecord record{};
    record.message_type = routed.message_type;
    record.features = std::move(features);
    record.signal = decision;
    record.trade = trade;
    record.portfolio = portfolioAfter;
    writer_.write(record);

    ++context.processed_messages;
    return true;
}

void AlphaEngine::finalize(std::ostream& out) {
    writer_.flush();
    io::RunSummary::print(out, stats_, contexts_);
}

const std::unordered_map<std::string, SymbolContext>& AlphaEngine::contexts() const {
    return contexts_;
}

const EngineStats& AlphaEngine::stats() const {
    return stats_;
}

}  // namespace apex::alpha::pipeline
