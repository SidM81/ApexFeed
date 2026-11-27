#include "ApexAlpha/Pipeline/SymbolRouter.h"

#include <algorithm>

namespace apex::alpha::pipeline {

namespace {

const Field* findField(const Field* fields, std::size_t count, TYPE type) {
    for (std::size_t i = 0; i < count; ++i) {
        if (fields[i].type == type) {
            return &fields[i];
        }
    }
    return nullptr;
}

std::string trimRight(std::string value) {
    while (!value.empty() && value.back() == ' ') {
        value.pop_back();
    }
    return value;
}

char messageType(const Field* fields, std::size_t count) {
    const Field* msgType = findField(fields, count, TYPE::MSGTYPE);
    if (msgType == nullptr || msgType->ascii_len == 0) {
        return '?';
    }
    return msgType->ascii[0];
}

std::uint64_t timestampValue(const Field* fields, std::size_t count) {
    const Field* ts = findField(fields, count, TYPE::TIMESTAMP);
    return ts == nullptr ? 0 : ts->u64;
}

}  // namespace

RoutedMessage SymbolRouter::route(const Field* fields, std::size_t count) {
    RoutedMessage routed{};
    routed.message_type = messageType(fields, count);
    routed.timestamp = timestampValue(fields, count);

    switch (routed.message_type) {
        case 'A':
        case 'F': {
            routed.book_event = true;
            const Field* orderRef = findField(fields, count, TYPE::ORDR_REF_NO);
            const Field* stock = findField(fields, count, TYPE::STCK);
            if (orderRef == nullptr || stock == nullptr) {
                return routed;
            }

            routed.symbol = trimRight(std::string(stock->ascii.data(), stock->ascii_len));
            if (routed.symbol.empty()) {
                return routed;
            }

            order_to_symbol_[orderRef->u64] = routed.symbol;
            routed.routed = true;
            return routed;
        }

        case 'E':
        case 'C':
        case 'X':
        case 'D': {
            routed.book_event = true;
            const Field* orderRef = findField(fields, count, TYPE::ORDR_REF_NO);
            if (orderRef == nullptr) {
                return routed;
            }

            const auto it = order_to_symbol_.find(orderRef->u64);
            if (it == order_to_symbol_.end()) {
                return routed;
            }

            routed.symbol = it->second;
            routed.routed = true;

            if (routed.message_type == 'D') {
                order_to_symbol_.erase(it);
            }
            return routed;
        }

        case 'U': {
            routed.book_event = true;
            const Field* oldOrder = findField(fields, count, TYPE::ORDR_REF_NO);
            const Field* newOrder = findField(fields, count, TYPE::NEW_ORDR_REF_NO);
            if (oldOrder == nullptr || newOrder == nullptr) {
                return routed;
            }

            const auto it = order_to_symbol_.find(oldOrder->u64);
            if (it == order_to_symbol_.end()) {
                return routed;
            }

            routed.symbol = it->second;
            routed.routed = true;
            order_to_symbol_.erase(it);
            order_to_symbol_[newOrder->u64] = routed.symbol;
            return routed;
        }

        default:
            return routed;
    }
}

}  // namespace apex::alpha::pipeline
