#ifndef APEXALPHA_MODELS_PORTFOLIO_STATE_H
#define APEXALPHA_MODELS_PORTFOLIO_STATE_H

#include <cstdint>
#include <string>

namespace apex::alpha {

struct PortfolioState {
    std::string symbol{};
    std::int64_t position{0};
    double avg_entry_price{0.0};
    double realized_pnl{0.0};
    double unrealized_pnl{0.0};
    double total_pnl{0.0};
};

}  // namespace apex::alpha

#endif
