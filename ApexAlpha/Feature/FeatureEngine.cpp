#include "ApexAlpha/Feature/FeatureEngine.h"

namespace apex::alpha::feature {

FeatureEngine::FeatureEngine(std::string symbol)
    : symbol_(std::move(symbol)) {}

FeatureSnapshot FeatureEngine::update(const apex::OrderBook::TopOfBook& topOfBook, std::uint64_t timestamp) {
    FeatureSnapshot snapshot{};
    snapshot.symbol = symbol_;
    snapshot.timestamp = timestamp;

    const auto bidVolume = topOfBook.has_bid ? topOfBook.best_bid_qty : 0;
    const auto askVolume = topOfBook.has_ask ? topOfBook.best_ask_qty : 0;

    if (state_.has_previous) {
        const double deltaBid = static_cast<double>(bidVolume) - static_cast<double>(state_.previous_bid_volume);
        const double deltaAsk = static_cast<double>(askVolume) - static_cast<double>(state_.previous_ask_volume);
        snapshot.order_flow_imbalance = deltaBid - deltaAsk;
    }

    state_.previous_bid_volume = bidVolume;
    state_.previous_ask_volume = askVolume;
    state_.has_previous = true;

    snapshot.has_valid_tob = topOfBook.has_bid && topOfBook.has_ask;
    if (!snapshot.has_valid_tob) {
        return snapshot;
    }

    const double bidPrice = static_cast<double>(topOfBook.best_bid_price);
    const double askPrice = static_cast<double>(topOfBook.best_ask_price);
    const double bidQty = static_cast<double>(topOfBook.best_bid_qty);
    const double askQty = static_cast<double>(topOfBook.best_ask_qty);

    snapshot.spread = askPrice - bidPrice;
    snapshot.mid = 0.5 * (bidPrice + askPrice);

    const double depth = bidQty + askQty;
    if (depth > 0.0) {
        snapshot.microprice = ((askPrice * bidQty) + (bidPrice * askQty)) / depth;
        snapshot.queue_imbalance = (bidQty - askQty) / depth;
    }

    return snapshot;
}

}  // namespace apex::alpha::feature
