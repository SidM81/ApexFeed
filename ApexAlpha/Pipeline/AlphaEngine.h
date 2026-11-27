#ifndef APEXALPHA_PIPELINE_ALPHA_ENGINE_H
#define APEXALPHA_PIPELINE_ALPHA_ENGINE_H

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>
#include <unordered_map>

#include "ApexAlpha/IO/CsvWriter.h"
#include "ApexAlpha/Pipeline/SymbolContext.h"
#include "ApexAlpha/Pipeline/SymbolRouter.h"
#include "ApexAlpha/Strategy/StrategyConfig.h"
#include "Parser/ITCHTypes.h"

namespace apex::alpha::pipeline {

struct EngineStats {
    std::uint64_t total_messages{0};
    std::uint64_t routed_messages{0};
    std::uint64_t unrouted_messages{0};
    std::uint64_t feature_updates{0};
    std::uint64_t signal_events{0};
    std::uint64_t trade_events{0};
};

class AlphaEngine {
public:
    AlphaEngine(strategy::StrategyConfig config, std::string outputPath);

    bool onMessage(const Field* fields, std::size_t count);
    void finalize(std::ostream& out);

    [[nodiscard]] const std::unordered_map<std::string, SymbolContext>& contexts() const;
    [[nodiscard]] const EngineStats& stats() const;

private:
    SymbolContext& getOrCreateContext(const std::string& symbol);

private:
    strategy::StrategyConfig config_{};
    SymbolRouter router_{};
    std::unordered_map<std::string, SymbolContext> contexts_{};
    io::CsvWriter writer_;
    EngineStats stats_{};
};

}  // namespace apex::alpha::pipeline

#endif
