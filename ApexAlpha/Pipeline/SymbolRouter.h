#ifndef APEXALPHA_PIPELINE_SYMBOL_ROUTER_H
#define APEXALPHA_PIPELINE_SYMBOL_ROUTER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "Parser/ITCHTypes.h"

namespace apex::alpha::pipeline {

struct RoutedMessage {
    bool routed{false};
    bool book_event{false};
    char message_type{'?'};
    std::string symbol{};
    std::uint64_t timestamp{0};
};

class SymbolRouter {
public:
    RoutedMessage route(const Field* fields, std::size_t count);

private:
    std::unordered_map<std::uint64_t, std::string> order_to_symbol_{};
};

}  // namespace apex::alpha::pipeline

#endif
