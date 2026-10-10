#include <iostream>

#include "ApexAlpha/Pipeline/SymbolRouter.h"

namespace {

Field makeMsgType(char type) {
    Field f{};
    f.type = TYPE::MSGTYPE;
    f.kind = FIELD_KIND::CHAR;
    f.setChar(type);
    return f;
}

Field makeTimestamp(std::uint64_t ts) {
    Field f{};
    f.type = TYPE::TIMESTAMP;
    f.kind = FIELD_KIND::UINT;
    f.u64 = ts;
    return f;
}

Field makeOrder(std::uint64_t id) {
    Field f{};
    f.type = TYPE::ORDR_REF_NO;
    f.kind = FIELD_KIND::UINT;
    f.u64 = id;
    return f;
}

Field makeNewOrder(std::uint64_t id) {
    Field f{};
    f.type = TYPE::NEW_ORDR_REF_NO;
    f.kind = FIELD_KIND::UINT;
    f.u64 = id;
    return f;
}

Field makeStock(const char* symbol) {
    Field f{};
    f.type = TYPE::STCK;
    f.kind = FIELD_KIND::ASCII;
    f.setAscii(symbol, 8);
    return f;
}

int fail(const char* message) {
    std::cerr << "[FAIL] " << message << '\n';
    return 1;
}

}  // namespace

int main() {
    apex::alpha::pipeline::SymbolRouter router;

    Field addFields[] = {
        makeMsgType('A'),
        makeTimestamp(10),
        makeOrder(1001),
        makeStock("AAPL    ")
    };

    const auto addRoute = router.route(addFields, 4);
    if (!addRoute.book_event || !addRoute.routed || addRoute.symbol != "AAPL") {
        return fail("failed to route add message");
    }

    Field execFields[] = {
        makeMsgType('E'),
        makeTimestamp(11),
        makeOrder(1001)
    };

    const auto execRoute = router.route(execFields, 3);
    if (!execRoute.routed || execRoute.symbol != "AAPL") {
        return fail("failed to route execute message");
    }

    Field replaceFields[] = {
        makeMsgType('U'),
        makeTimestamp(12),
        makeOrder(1001),
        makeNewOrder(2002)
    };

    const auto replaceRoute = router.route(replaceFields, 4);
    if (!replaceRoute.routed || replaceRoute.symbol != "AAPL") {
        return fail("failed to route replace message");
    }

    Field deleteFields[] = {
        makeMsgType('D'),
        makeTimestamp(13),
        makeOrder(2002)
    };

    const auto deleteRoute = router.route(deleteFields, 3);
    if (!deleteRoute.routed || deleteRoute.symbol != "AAPL") {
        return fail("failed to route delete message");
    }

    Field postDeleteExec[] = {
        makeMsgType('E'),
        makeTimestamp(14),
        makeOrder(2002)
    };

    const auto postDeleteRoute = router.route(postDeleteExec, 3);
    if (postDeleteRoute.routed) {
        return fail("route should not resolve deleted order");
    }

    std::cout << "[PASS] ApexAlpha symbol router\n";
    return 0;
}
