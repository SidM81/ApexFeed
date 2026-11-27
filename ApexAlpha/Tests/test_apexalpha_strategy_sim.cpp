#include <iostream>

#include "ApexAlpha/Simulation/ExecutionSimulator.h"
#include "ApexAlpha/Simulation/PnlTracker.h"
#include "ApexAlpha/Strategy/ThresholdStrategy.h"

namespace {

int fail(const char* message) {
    std::cerr << "[FAIL] " << message << '\n';
    return 1;
}

}  // namespace

int main() {
    apex::alpha::strategy::StrategyConfig config{};
    config.order_size = 100;
    config.max_position = 1000;
    config.buy_qi_threshold = 0.1;
    config.sell_qi_threshold = -0.1;
    config.buy_ofi_threshold = 1.0;
    config.sell_ofi_threshold = -1.0;
    config.exit_long_qi_threshold = -0.05;
    config.exit_long_ofi_threshold = -0.5;
    config.exit_short_qi_threshold = 0.05;
    config.exit_short_ofi_threshold = 0.5;

    apex::alpha::strategy::ThresholdStrategy strategy(config);
    apex::alpha::simulation::ExecutionSimulator simulator("AAPL", config.max_position);
    apex::alpha::simulation::PnlTracker pnl("AAPL");

    apex::OrderBook::TopOfBook top{};
    top.has_bid = true;
    top.has_ask = true;
    top.best_bid_price = 100;
    top.best_bid_qty = 120;
    top.best_ask_price = 101;
    top.best_ask_qty = 80;

    apex::alpha::FeatureSnapshot bullish{};
    bullish.symbol = "AAPL";
    bullish.timestamp = 1;
    bullish.has_valid_tob = true;
    bullish.mid = 100.5;
    bullish.queue_imbalance = 0.40;
    bullish.order_flow_imbalance = 5.0;

    auto portfolio = pnl.snapshot();
    auto decision = strategy.onFeatures(bullish, portfolio);
    if (decision.side != apex::alpha::SignalSide::BUY) {
        return fail("expected BUY signal");
    }

    auto trade = simulator.apply(decision, top, bullish.timestamp);
    if (!trade.has_value() || trade->side != apex::alpha::TradeSide::BUY || trade->price != 101.0) {
        return fail("expected BUY trade at ask");
    }

    pnl.onTrade(*trade);
    pnl.markToMarket(102.0);

    portfolio = pnl.snapshot();
    if (portfolio.position != 100) {
        return fail("position should be long 100");
    }

    apex::alpha::FeatureSnapshot bearish{};
    bearish.symbol = "AAPL";
    bearish.timestamp = 2;
    bearish.has_valid_tob = true;
    bearish.mid = 100.0;
    bearish.queue_imbalance = -0.20;
    bearish.order_flow_imbalance = -2.0;

    decision = strategy.onFeatures(bearish, portfolio);
    if (decision.side != apex::alpha::SignalSide::EXIT) {
        return fail("expected EXIT signal");
    }

    trade = simulator.apply(decision, top, bearish.timestamp);
    if (!trade.has_value() || trade->side != apex::alpha::TradeSide::SELL || trade->size != 100 || trade->price != 100.0) {
        return fail("expected SELL exit trade at bid");
    }

    pnl.onTrade(*trade);
    pnl.markToMarket(100.0);

    portfolio = pnl.snapshot();
    if (portfolio.position != 0) {
        return fail("position should be flat after exit");
    }

    if (portfolio.realized_pnl >= 0.0) {
        return fail("expected negative realized pnl after buying high and selling low");
    }

    std::cout << "[PASS] ApexAlpha strategy and simulation\n";
    return 0;
}
