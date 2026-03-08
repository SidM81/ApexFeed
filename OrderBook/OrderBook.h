#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "Parser/ITCHTypes.h"

namespace apex {

class OrderBook {
public:
    struct TopOfBook {
        bool has_bid{false};
        bool has_ask{false};
        std::uint32_t best_bid_price{0};
        std::uint64_t best_bid_qty{0};
        std::uint32_t best_ask_price{0};
        std::uint64_t best_ask_qty{0};
    };

    void applyITCH(const Field* fields, std::size_t count);

    void addOrder(std::uint64_t orderId, char side, std::uint32_t qty, std::uint32_t price);
    void executeOrder(std::uint64_t orderId, std::uint32_t executedQty);
    void cancelOrder(std::uint64_t orderId, std::uint32_t canceledQty);
    void deleteOrder(std::uint64_t orderId);
    void replaceOrder(std::uint64_t oldOrderId,
                      std::uint64_t newOrderId,
                      std::uint32_t newQty,
                      std::uint32_t newPrice);

    [[nodiscard]] TopOfBook topOfBook() const;
    [[nodiscard]] std::size_t orderCount() const;

private:
    struct PriceLevel {
        std::uint32_t price{0};
        std::uint64_t total_qty{0};
        std::uint32_t order_count{0};
        PriceLevel* prev{nullptr};
        PriceLevel* next{nullptr};
    };

    struct OrderRecord {
        std::uint64_t id{0};
        std::uint32_t price{0};
        std::uint32_t qty{0};
        char side{'B'};
        PriceLevel* level{nullptr};
    };

    static const Field* findField(const Field* fields, std::size_t count, TYPE type);

    PriceLevel* getOrCreateLevel(char side, std::uint32_t price);
    void removeLevel(char side, PriceLevel* level);
    void reduceOrder(std::uint64_t orderId, std::uint32_t qty, bool deleteCompletely);

private:
    PriceLevel* best_bid_{nullptr};
    PriceLevel* best_ask_{nullptr};

    std::unordered_map<std::uint32_t, std::unique_ptr<PriceLevel>> bid_levels_;
    std::unordered_map<std::uint32_t, std::unique_ptr<PriceLevel>> ask_levels_;
    std::unordered_map<std::uint64_t, OrderRecord> orders_;
};

}  // namespace apex

#endif
