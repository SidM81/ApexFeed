#ifndef APEXALPHA_MODELS_EVENT_RECORD_H
#define APEXALPHA_MODELS_EVENT_RECORD_H

#include <optional>

#include "ApexAlpha/Models/Features.h"
#include "ApexAlpha/Models/PortfolioState.h"
#include "ApexAlpha/Models/Signal.h"
#include "ApexAlpha/Models/Trade.h"

namespace apex::alpha {

struct EventRecord {
    char message_type{'?'};
    FeatureSnapshot features{};
    SignalDecision signal{};
    std::optional<Trade> trade{};
    PortfolioState portfolio{};
};

}  // namespace apex::alpha

#endif
