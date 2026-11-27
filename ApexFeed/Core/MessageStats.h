#ifndef CORE_MESSAGE_STATS_H
#define CORE_MESSAGE_STATS_H

#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>

namespace apex {

class MessageStats {
public:
    explicit MessageStats(std::uint64_t reportEvery = 100000)
        : report_every_(reportEvery), start_(Clock::now()), last_report_(start_) {
        counts_by_type_.fill(0);
    }

    void onMessage(char type, std::uint64_t parseNs) {
        const unsigned char idx = static_cast<unsigned char>(type);
        ++total_messages_;
        ++counts_by_type_[idx];
        parse_ns_total_ += parseNs;

        if (report_every_ > 0 && (total_messages_ % report_every_ == 0)) {
            report(false);
        }
    }

    void onBatch(const std::array<std::uint64_t, 256>& typeCounts,
                 std::uint64_t batchMessages,
                 std::uint64_t batchParseNs) {
        if (batchMessages == 0) {
            return;
        }

        const std::uint64_t prev = total_messages_;
        total_messages_ += batchMessages;
        parse_ns_total_ += batchParseNs;

        for (std::size_t i = 0; i < counts_by_type_.size(); ++i) {
            counts_by_type_[i] += typeCounts[i];
        }

        if (report_every_ > 0) {
            const std::uint64_t prevBucket = prev / report_every_;
            const std::uint64_t nextBucket = total_messages_ / report_every_;
            if (nextBucket > prevBucket) {
                report(false);
            }
        }
    }

    void report(bool force) {
        if (!force && report_every_ == 0) {
            return;
        }

        const auto now = Clock::now();
        const double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - start_).count();
        const double mps = elapsed > 0.0 ? static_cast<double>(total_messages_) / elapsed : 0.0;
        const double avg_parse_ns = total_messages_ > 0 ? static_cast<double>(parse_ns_total_) / static_cast<double>(total_messages_) : 0.0;

        std::cout << "[PERF] messages=" << total_messages_
                  << " time=" << elapsed
                  << " msg/sec=" << mps
                  << " avg_parse_ns=" << avg_parse_ns
                  << '\n';

        last_report_ = now;
    }

    void printTypeBreakdown() const {
        std::cout << "[STATS] message-type-counts";
        for (std::size_t i = 0; i < counts_by_type_.size(); ++i) {
            if (counts_by_type_[i] == 0) {
                continue;
            }
            const char type = static_cast<char>(i);
            std::cout << ' ' << type << '=' << counts_by_type_[i];
        }
        std::cout << '\n';
    }

    [[nodiscard]] std::uint64_t totalMessages() const { return total_messages_; }

private:
    using Clock = std::chrono::steady_clock;

    std::array<std::uint64_t, 256> counts_by_type_{};
    std::uint64_t total_messages_{0};
    std::uint64_t parse_ns_total_{0};
    std::uint64_t report_every_{0};
    Clock::time_point start_;
    Clock::time_point last_report_;
};

}  // namespace apex

#endif
