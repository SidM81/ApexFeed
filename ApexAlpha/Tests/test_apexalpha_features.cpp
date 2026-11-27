#include <cmath>
#include <iostream>

#include "ApexAlpha/Feature/FeatureEngine.h"

namespace {

bool approx(double lhs, double rhs, double eps = 1e-9) {
    return std::fabs(lhs - rhs) <= eps;
}

int fail(const char* message) {
    std::cerr << "[FAIL] " << message << '\n';
    return 1;
}

}  // namespace

int main() {
    apex::alpha::feature::FeatureEngine engine("AAPL");

    apex::OrderBook::TopOfBook top{};
    top.has_bid = true;
    top.has_ask = true;
    top.best_bid_price = 100;
    top.best_bid_qty = 100;
    top.best_ask_price = 101;
    top.best_ask_qty = 100;

    const auto first = engine.update(top, 1);
    if (!first.has_valid_tob) {
        return fail("first snapshot must have valid top of book");
    }
    if (!approx(first.spread, 1.0)) {
        return fail("spread mismatch");
    }
    if (!approx(first.mid, 100.5)) {
        return fail("mid mismatch");
    }
    if (!approx(first.microprice, 100.5)) {
        return fail("microprice mismatch");
    }
    if (!approx(first.queue_imbalance, 0.0)) {
        return fail("queue imbalance mismatch");
    }
    if (!approx(first.order_flow_imbalance, 0.0)) {
        return fail("first OFI must be zero");
    }

    top.best_bid_qty = 120;
    top.best_ask_qty = 80;

    const auto second = engine.update(top, 2);
    if (!approx(second.order_flow_imbalance, 40.0)) {
        return fail("OFI mismatch on second update");
    }

    const double expectedMicro = ((101.0 * 120.0) + (100.0 * 80.0)) / 200.0;
    const double expectedQi = (120.0 - 80.0) / 200.0;

    if (!approx(second.microprice, expectedMicro)) {
        return fail("microprice mismatch on second update");
    }
    if (!approx(second.queue_imbalance, expectedQi)) {
        return fail("queue imbalance mismatch on second update");
    }

    std::cout << "[PASS] ApexAlpha feature engine\n";
    return 0;
}
