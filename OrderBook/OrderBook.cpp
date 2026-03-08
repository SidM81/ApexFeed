#include "OrderBook.h"

#include <algorithm>

namespace apex {

const Field* OrderBook::findField(const Field* fields, std::size_t count, TYPE type) {
    for (std::size_t i = 0; i < count; ++i) {
        if (fields[i].type == type) {
            return &fields[i];
        }
    }
    return nullptr;
}

OrderBook::PriceLevel* OrderBook::getOrCreateLevel(char side, std::uint32_t price) {
    const bool isBid = side == 'B';
    auto& levels = isBid ? bid_levels_ : ask_levels_;
    auto found = levels.find(price);
    if (found != levels.end()) {
        return found->second.get();
    }

    auto level = std::make_unique<PriceLevel>();
    level->price = price;
    PriceLevel* levelPtr = level.get();
    levels.emplace(price, std::move(level));

    PriceLevel*& head = isBid ? best_bid_ : best_ask_;
    if (head == nullptr) {
        head = levelPtr;
        return levelPtr;
    }

    if (isBid) {
        if (price > head->price) {
            levelPtr->next = head;
            head->prev = levelPtr;
            head = levelPtr;
            return levelPtr;
        }

        PriceLevel* cur = head;
        while (cur->next != nullptr && cur->next->price >= price) {
            cur = cur->next;
        }

        levelPtr->next = cur->next;
        if (cur->next != nullptr) {
            cur->next->prev = levelPtr;
        }
        cur->next = levelPtr;
        levelPtr->prev = cur;
        return levelPtr;
    }

    if (price < head->price) {
        levelPtr->next = head;
        head->prev = levelPtr;
        head = levelPtr;
        return levelPtr;
    }

    PriceLevel* cur = head;
    while (cur->next != nullptr && cur->next->price <= price) {
        cur = cur->next;
    }

    levelPtr->next = cur->next;
    if (cur->next != nullptr) {
        cur->next->prev = levelPtr;
    }
    cur->next = levelPtr;
    levelPtr->prev = cur;
    return levelPtr;
}

void OrderBook::removeLevel(char side, PriceLevel* level) {
    if (level == nullptr) {
        return;
    }

    PriceLevel*& head = (side == 'B') ? best_bid_ : best_ask_;
    if (level == head) {
        head = level->next;
    }

    if (level->prev != nullptr) {
        level->prev->next = level->next;
    }
    if (level->next != nullptr) {
        level->next->prev = level->prev;
    }

    level->prev = nullptr;
    level->next = nullptr;

    if (side == 'B') {
        bid_levels_.erase(level->price);
    } else {
        ask_levels_.erase(level->price);
    }
}

void OrderBook::addOrder(std::uint64_t orderId, char side, std::uint32_t qty, std::uint32_t price) {
    if (qty == 0) {
        return;
    }

    const auto existing = orders_.find(orderId);
    if (existing != orders_.end()) {
        deleteOrder(orderId);
    }

    PriceLevel* level = getOrCreateLevel(side, price);
    level->total_qty += qty;
    level->order_count += 1;

    OrderRecord rec{};
    rec.id = orderId;
    rec.price = price;
    rec.qty = qty;
    rec.side = side;
    rec.level = level;

    orders_.emplace(orderId, rec);
}

void OrderBook::reduceOrder(std::uint64_t orderId, std::uint32_t qty, bool deleteCompletely) {
    auto it = orders_.find(orderId);
    if (it == orders_.end()) {
        return;
    }

    OrderRecord& order = it->second;
    const std::uint32_t reduction = deleteCompletely ? order.qty : std::min(order.qty, qty);
    if (reduction == 0) {
        return;
    }

    order.qty -= reduction;
    if (order.level->total_qty >= reduction) {
        order.level->total_qty -= reduction;
    } else {
        order.level->total_qty = 0;
    }

    if (order.qty == 0 || deleteCompletely) {
        if (order.level->order_count > 0) {
            order.level->order_count -= 1;
        }

        PriceLevel* level = order.level;
        const char side = order.side;
        orders_.erase(it);

        if (level->order_count == 0 || level->total_qty == 0) {
            removeLevel(side, level);
        }
    }
}

void OrderBook::executeOrder(std::uint64_t orderId, std::uint32_t executedQty) {
    reduceOrder(orderId, executedQty, false);
}

void OrderBook::cancelOrder(std::uint64_t orderId, std::uint32_t canceledQty) {
    reduceOrder(orderId, canceledQty, false);
}

void OrderBook::deleteOrder(std::uint64_t orderId) {
    reduceOrder(orderId, 0, true);
}

void OrderBook::replaceOrder(std::uint64_t oldOrderId,
                             std::uint64_t newOrderId,
                             std::uint32_t newQty,
                             std::uint32_t newPrice) {
    auto it = orders_.find(oldOrderId);
    if (it == orders_.end()) {
        return;
    }

    const char side = it->second.side;
    deleteOrder(oldOrderId);
    addOrder(newOrderId, side, newQty, newPrice);
}

void OrderBook::applyITCH(const Field* fields, std::size_t count) {
    const Field* msgType = findField(fields, count, TYPE::MSGTYPE);
    if (msgType == nullptr || msgType->ascii_len == 0) {
        return;
    }

    const char type = msgType->ascii[0];

    switch (type) {
        case 'A':
        case 'F': {
            const Field* order = findField(fields, count, TYPE::ORDR_REF_NO);
            const Field* side = findField(fields, count, TYPE::SIDE);
            const Field* qty = findField(fields, count, TYPE::QNTY);
            const Field* price = findField(fields, count, TYPE::PRICE);
            if (order && side && qty && price && side->ascii_len == 1) {
                addOrder(order->u64,
                         side->ascii[0],
                         static_cast<std::uint32_t>(qty->u64),
                         static_cast<std::uint32_t>(price->u64));
            }
            break;
        }
        case 'E':
        case 'C': {
            const Field* order = findField(fields, count, TYPE::ORDR_REF_NO);
            const Field* executed = findField(fields, count, TYPE::EXEC_SHARES);
            if (order && executed) {
                executeOrder(order->u64, static_cast<std::uint32_t>(executed->u64));
            }
            break;
        }
        case 'X': {
            const Field* order = findField(fields, count, TYPE::ORDR_REF_NO);
            const Field* canceled = findField(fields, count, TYPE::CNCLD_SHARES);
            if (order && canceled) {
                cancelOrder(order->u64, static_cast<std::uint32_t>(canceled->u64));
            }
            break;
        }
        case 'D': {
            const Field* order = findField(fields, count, TYPE::ORDR_REF_NO);
            if (order) {
                deleteOrder(order->u64);
            }
            break;
        }
        case 'U': {
            const Field* oldRef = findField(fields, count, TYPE::ORDR_REF_NO);
            const Field* newRef = findField(fields, count, TYPE::NEW_ORDR_REF_NO);
            const Field* qty = findField(fields, count, TYPE::QNTY);
            const Field* price = findField(fields, count, TYPE::PRICE);
            if (oldRef && newRef && qty && price) {
                replaceOrder(oldRef->u64,
                             newRef->u64,
                             static_cast<std::uint32_t>(qty->u64),
                             static_cast<std::uint32_t>(price->u64));
            }
            break;
        }
        default:
            break;
    }
}

OrderBook::TopOfBook OrderBook::topOfBook() const {
    TopOfBook tob{};
    if (best_bid_ != nullptr) {
        tob.has_bid = true;
        tob.best_bid_price = best_bid_->price;
        tob.best_bid_qty = best_bid_->total_qty;
    }

    if (best_ask_ != nullptr) {
        tob.has_ask = true;
        tob.best_ask_price = best_ask_->price;
        tob.best_ask_qty = best_ask_->total_qty;
    }

    return tob;
}

std::size_t OrderBook::orderCount() const {
    return orders_.size();
}

}  // namespace apex
