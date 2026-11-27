#ifndef APEXALPHA_SIMULATION_EXECUTION_SIMULATOR_H
#define APEXALPHA_SIMULATION_EXECUTION_SIMULATOR_H

#include <cstdint>
#include <optional>
#include <string>

#include "ApexAlpha/Models/Signal.h"
#include "ApexAlpha/Models/Trade.h"
#include "OrderBook/OrderBook.h"

namespace apex::alpha::simulation {

class ExecutionSimulator {
public:
    ExecutionSimulator(std::string symbol, std::int64_t maxPosition);

    std::optional<Trade> apply(const SignalDecision& decision,
                               const apex::OrderBook::TopOfBook& topOfBook,
                               std::uint64_t timestamp);

    [[nodiscard]] std::int64_t currentPosition() const;

private:
    std::string symbol_{};
    std::int64_t position_{0};
    std::int64_t max_position_{0};
};

}  // namespace apex::alpha::simulation

#endif
