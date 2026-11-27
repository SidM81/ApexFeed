#include "ApexAlpha/Simulation/PnlTracker.h"

#include <algorithm>
#include <cstdint>
#include <utility>

namespace apex::alpha::simulation {

PnlTracker::PnlTracker(std::string symbol)
    : symbol_(std::move(symbol)) {}

void PnlTracker::onTrade(const Trade& trade) {
    const std::int64_t size = static_cast<std::int64_t>(trade.size);
    if (size <= 0) {
        return;
    }

    const double price = trade.price;

    if (trade.side == TradeSide::BUY) {
        if (position_ >= 0) {
            const double gross = (avg_entry_price_ * static_cast<double>(position_)) + (price * static_cast<double>(size));
            position_ += size;
            avg_entry_price_ = position_ > 0 ? gross / static_cast<double>(position_) : 0.0;
        } else {
            const std::int64_t cover = std::min<std::int64_t>(size, -position_);
            realized_pnl_ += (avg_entry_price_ - price) * static_cast<double>(cover);
            position_ += cover;

            const std::int64_t residualBuy = size - cover;
            if (residualBuy > 0) {
                position_ += residualBuy;
                avg_entry_price_ = price;
            } else if (position_ == 0) {
                avg_entry_price_ = 0.0;
            }
        }
    } else {
        if (position_ <= 0) {
            const double gross = (avg_entry_price_ * static_cast<double>(-position_)) + (price * static_cast<double>(size));
            position_ -= size;
            avg_entry_price_ = position_ < 0 ? gross / static_cast<double>(-position_) : 0.0;
        } else {
            const std::int64_t close = std::min<std::int64_t>(size, position_);
            realized_pnl_ += (price - avg_entry_price_) * static_cast<double>(close);
            position_ -= close;

            const std::int64_t residualSell = size - close;
            if (residualSell > 0) {
                position_ -= residualSell;
                avg_entry_price_ = price;
            } else if (position_ == 0) {
                avg_entry_price_ = 0.0;
            }
        }
    }

    if (position_ == 0) {
        avg_entry_price_ = 0.0;
    }
}

void PnlTracker::markToMarket(double midPrice) {
    if (midPrice > 0.0) {
        last_mid_ = midPrice;
    }

    if (position_ > 0) {
        unrealized_pnl_ = (last_mid_ - avg_entry_price_) * static_cast<double>(position_);
    } else if (position_ < 0) {
        unrealized_pnl_ = (avg_entry_price_ - last_mid_) * static_cast<double>(-position_);
    } else {
        unrealized_pnl_ = 0.0;
    }
}

PortfolioState PnlTracker::snapshot() const {
    PortfolioState state{};
    state.symbol = symbol_;
    state.position = position_;
    state.avg_entry_price = avg_entry_price_;
    state.realized_pnl = realized_pnl_;
    state.unrealized_pnl = unrealized_pnl_;
    state.total_pnl = realized_pnl_ + unrealized_pnl_;
    return state;
}

}  // namespace apex::alpha::simulation
