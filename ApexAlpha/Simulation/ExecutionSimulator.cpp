#include "ApexAlpha/Simulation/ExecutionSimulator.h"

#include <algorithm>
#include <utility>

namespace apex::alpha::simulation {

ExecutionSimulator::ExecutionSimulator(std::string symbol, std::int64_t maxPosition)
    : symbol_(std::move(symbol)),
      max_position_(maxPosition > 0 ? maxPosition : 0) {}

std::optional<Trade> ExecutionSimulator::apply(const SignalDecision& decision,
                                               const apex::OrderBook::TopOfBook& topOfBook,
                                               std::uint64_t timestamp) {
    std::uint32_t size = decision.size;
    TradeSide side = TradeSide::BUY;
    double price = 0.0;

    switch (decision.side) {
        case SignalSide::HOLD:
            return std::nullopt;
        case SignalSide::EXIT:
            if (position_ == 0) {
                return std::nullopt;
            }
            side = position_ > 0 ? TradeSide::SELL : TradeSide::BUY;
            size = static_cast<std::uint32_t>(std::llabs(position_));
            break;
        case SignalSide::BUY:
            side = TradeSide::BUY;
            break;
        case SignalSide::SELL:
            side = TradeSide::SELL;
            break;
    }

    if (size == 0) {
        return std::nullopt;
    }

    if (side == TradeSide::BUY) {
        if (!topOfBook.has_ask) {
            return std::nullopt;
        }

        const std::int64_t allowed = max_position_ - position_;
        if (allowed <= 0) {
            return std::nullopt;
        }

        size = static_cast<std::uint32_t>(std::min<std::int64_t>(allowed, size));
        price = static_cast<double>(topOfBook.best_ask_price);
        position_ += static_cast<std::int64_t>(size);
    } else {
        if (!topOfBook.has_bid) {
            return std::nullopt;
        }

        const std::int64_t allowed = max_position_ + position_;
        if (allowed <= 0) {
            return std::nullopt;
        }

        size = static_cast<std::uint32_t>(std::min<std::int64_t>(allowed, size));
        price = static_cast<double>(topOfBook.best_bid_price);
        position_ -= static_cast<std::int64_t>(size);
    }

    if (size == 0) {
        return std::nullopt;
    }

    Trade trade{};
    trade.timestamp = timestamp;
    trade.symbol = symbol_;
    trade.side = side;
    trade.size = size;
    trade.price = price;
    return trade;
}

std::int64_t ExecutionSimulator::currentPosition() const {
    return position_;
}

}  // namespace apex::alpha::simulation
