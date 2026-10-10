#ifndef APEXALPHA_IO_RUN_SUMMARY_H
#define APEXALPHA_IO_RUN_SUMMARY_H

#include <iosfwd>
#include <string>
#include <unordered_map>

#include "ApexAlpha/Pipeline/AlphaEngine.h"
#include "ApexAlpha/Pipeline/SymbolContext.h"

namespace apex::alpha::io {

class RunSummary {
public:
    static void print(std::ostream& out,
                      const pipeline::EngineStats& stats,
                      const std::unordered_map<std::string, pipeline::SymbolContext>& contexts);
};

}  // namespace apex::alpha::io

#endif
