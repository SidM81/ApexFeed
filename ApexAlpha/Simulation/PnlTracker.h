#ifndef APEXALPHA_SIMULATION_PNL_TRACKER_H
#define APEXALPHA_SIMULATION_PNL_TRACKER_H

#include <cstdint>
#include <string>

#include "ApexAlpha/Models/PortfolioState.h"
#include "ApexAlpha/Models/Trade.h"

namespace apex::alpha::simulation {

class PnlTracker {
public:
    explicit PnlTracker(std::string symbol);

    void onTrade(const Trade& trade);
    void markToMarket(double midPrice);

    [[nodiscard]] PortfolioState snapshot() const;

private:
    std::string symbol_{};
    std::int64_t position_{0};
    double avg_entry_price_{0.0};
    double realized_pnl_{0.0};
    double unrealized_pnl_{0.0};
    double last_mid_{0.0};
};

}  // namespace apex::alpha::simulation

#endif
