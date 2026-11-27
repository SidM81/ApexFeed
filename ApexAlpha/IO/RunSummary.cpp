#include "ApexAlpha/IO/RunSummary.h"

#include <algorithm>
#include <iomanip>
#include <vector>

namespace apex::alpha::io {

void RunSummary::print(std::ostream& out,
                       const pipeline::EngineStats& stats,
                       const std::unordered_map<std::string, pipeline::SymbolContext>& contexts) {
    out << std::fixed << std::setprecision(6);
    out << "[ALPHA] total_messages=" << stats.total_messages
        << " routed_messages=" << stats.routed_messages
        << " unrouted_messages=" << stats.unrouted_messages
        << " feature_updates=" << stats.feature_updates
        << " signal_events=" << stats.signal_events
        << " trade_events=" << stats.trade_events
        << '\n';

    std::vector<std::string> symbols;
    symbols.reserve(contexts.size());
    for (const auto& [symbol, context] : contexts) {
        (void)context;
        symbols.push_back(symbol);
    }
    std::sort(symbols.begin(), symbols.end());

    double realizedTotal = 0.0;
    double unrealizedTotal = 0.0;
    double pnlTotal = 0.0;

    for (const auto& symbol : symbols) {
        const auto it = contexts.find(symbol);
        if (it == contexts.end()) {
            continue;
        }

        const auto portfolio = it->second.pnl.snapshot();
        realizedTotal += portfolio.realized_pnl;
        unrealizedTotal += portfolio.unrealized_pnl;
        pnlTotal += portfolio.total_pnl;

        out << "[ALPHA][SYMBOL] symbol=" << symbol
            << " messages=" << it->second.processed_messages
            << " trades=" << it->second.trade_count
            << " position=" << portfolio.position
            << " realized=" << portfolio.realized_pnl
            << " unrealized=" << portfolio.unrealized_pnl
            << " total=" << portfolio.total_pnl
            << '\n';
    }

    out << "[ALPHA][TOTAL] symbols=" << symbols.size()
        << " realized=" << realizedTotal
        << " unrealized=" << unrealizedTotal
        << " total=" << pnlTotal
        << '\n';
}

}  // namespace apex::alpha::io
