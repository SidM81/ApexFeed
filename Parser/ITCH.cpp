#include "ITCH.h"

#include <algorithm>
#include <cstring>

#if defined(__GNUC__) || defined(__clang__)
#define APEX_LIKELY(x) (__builtin_expect(!!(x), 1))
#define APEX_UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define APEX_LIKELY(x) (x)
#define APEX_UNLIKELY(x) (x)
#endif

namespace {

struct FieldSpec {
    TYPE type;
    std::uint8_t offset;
    std::uint8_t length;
    FIELD_KIND kind;
};

constexpr std::array<FieldSpec, 9> kAddOrderFields{{
    {TYPE::MSGTYPE, 0, 1, FIELD_KIND::CHAR},
    {TYPE::STCK_LOC, 1, 2, FIELD_KIND::UINT},
    {TYPE::TRCK_NO, 3, 2, FIELD_KIND::UINT},
    {TYPE::TIMESTAMP, 5, 6, FIELD_KIND::UINT},
    {TYPE::ORDR_REF_NO, 11, 8, FIELD_KIND::UINT},
    {TYPE::SIDE, 19, 1, FIELD_KIND::CHAR},
    {TYPE::QNTY, 20, 4, FIELD_KIND::UINT},
    {TYPE::STCK, 24, 8, FIELD_KIND::ASCII},
    {TYPE::PRICE, 32, 4, FIELD_KIND::PRICE},
}};

constexpr std::array<FieldSpec, 10> kAddOrderWithMpidFields{{
    {TYPE::MSGTYPE, 0, 1, FIELD_KIND::CHAR},
    {TYPE::STCK_LOC, 1, 2, FIELD_KIND::UINT},
    {TYPE::TRCK_NO, 3, 2, FIELD_KIND::UINT},
    {TYPE::TIMESTAMP, 5, 6, FIELD_KIND::UINT},
    {TYPE::ORDR_REF_NO, 11, 8, FIELD_KIND::UINT},
    {TYPE::SIDE, 19, 1, FIELD_KIND::CHAR},
    {TYPE::QNTY, 20, 4, FIELD_KIND::UINT},
    {TYPE::STCK, 24, 8, FIELD_KIND::ASCII},
    {TYPE::PRICE, 32, 4, FIELD_KIND::PRICE},
    {TYPE::ATTRBTN, 36, 4, FIELD_KIND::ASCII},
}};

constexpr std::array<FieldSpec, 7> kOrderExecutedFields{{
    {TYPE::MSGTYPE, 0, 1, FIELD_KIND::CHAR},
    {TYPE::STCK_LOC, 1, 2, FIELD_KIND::UINT},
    {TYPE::TRCK_NO, 3, 2, FIELD_KIND::UINT},
    {TYPE::TIMESTAMP, 5, 6, FIELD_KIND::UINT},
    {TYPE::ORDR_REF_NO, 11, 8, FIELD_KIND::UINT},
    {TYPE::EXEC_SHARES, 19, 4, FIELD_KIND::UINT},
    {TYPE::MATCH_NO, 23, 8, FIELD_KIND::UINT},
}};

constexpr std::array<FieldSpec, 9> kOrderExecutedPriceFields{{
    {TYPE::MSGTYPE, 0, 1, FIELD_KIND::CHAR},
    {TYPE::STCK_LOC, 1, 2, FIELD_KIND::UINT},
    {TYPE::TRCK_NO, 3, 2, FIELD_KIND::UINT},
    {TYPE::TIMESTAMP, 5, 6, FIELD_KIND::UINT},
    {TYPE::ORDR_REF_NO, 11, 8, FIELD_KIND::UINT},
    {TYPE::EXEC_SHARES, 19, 4, FIELD_KIND::UINT},
    {TYPE::MATCH_NO, 23, 8, FIELD_KIND::UINT},
    {TYPE::PRNTABLE, 31, 1, FIELD_KIND::CHAR},
    {TYPE::EXEC_PRICE, 32, 4, FIELD_KIND::PRICE},
}};

constexpr std::array<FieldSpec, 6> kOrderCancelFields{{
    {TYPE::MSGTYPE, 0, 1, FIELD_KIND::CHAR},
    {TYPE::STCK_LOC, 1, 2, FIELD_KIND::UINT},
    {TYPE::TRCK_NO, 3, 2, FIELD_KIND::UINT},
    {TYPE::TIMESTAMP, 5, 6, FIELD_KIND::UINT},
    {TYPE::ORDR_REF_NO, 11, 8, FIELD_KIND::UINT},
    {TYPE::CNCLD_SHARES, 19, 4, FIELD_KIND::UINT},
}};

constexpr std::array<FieldSpec, 5> kOrderDeleteFields{{
    {TYPE::MSGTYPE, 0, 1, FIELD_KIND::CHAR},
    {TYPE::STCK_LOC, 1, 2, FIELD_KIND::UINT},
    {TYPE::TRCK_NO, 3, 2, FIELD_KIND::UINT},
    {TYPE::TIMESTAMP, 5, 6, FIELD_KIND::UINT},
    {TYPE::ORDR_REF_NO, 11, 8, FIELD_KIND::UINT},
}};

constexpr std::array<FieldSpec, 8> kOrderReplaceFields{{
    {TYPE::MSGTYPE, 0, 1, FIELD_KIND::CHAR},
    {TYPE::STCK_LOC, 1, 2, FIELD_KIND::UINT},
    {TYPE::TRCK_NO, 3, 2, FIELD_KIND::UINT},
    {TYPE::TIMESTAMP, 5, 6, FIELD_KIND::UINT},
    {TYPE::ORDR_REF_NO, 11, 8, FIELD_KIND::UINT},
    {TYPE::NEW_ORDR_REF_NO, 19, 8, FIELD_KIND::UINT},
    {TYPE::QNTY, 27, 4, FIELD_KIND::UINT},
    {TYPE::PRICE, 31, 4, FIELD_KIND::PRICE},
}};

constexpr std::array<FieldSpec, 10> kTradeFields{{
    {TYPE::MSGTYPE, 0, 1, FIELD_KIND::CHAR},
    {TYPE::STCK_LOC, 1, 2, FIELD_KIND::UINT},
    {TYPE::TRCK_NO, 3, 2, FIELD_KIND::UINT},
    {TYPE::TIMESTAMP, 5, 6, FIELD_KIND::UINT},
    {TYPE::ORDR_REF_NO, 11, 8, FIELD_KIND::UINT},
    {TYPE::SIDE, 19, 1, FIELD_KIND::CHAR},
    {TYPE::QNTY, 20, 4, FIELD_KIND::UINT},
    {TYPE::STCK, 24, 8, FIELD_KIND::ASCII},
    {TYPE::PRICE, 32, 4, FIELD_KIND::PRICE},
    {TYPE::MATCH_NO, 36, 8, FIELD_KIND::UINT},
}};

constexpr std::array<FieldSpec, 9> kCrossTradeFields{{
    {TYPE::MSGTYPE, 0, 1, FIELD_KIND::CHAR},
    {TYPE::STCK_LOC, 1, 2, FIELD_KIND::UINT},
    {TYPE::TRCK_NO, 3, 2, FIELD_KIND::UINT},
    {TYPE::TIMESTAMP, 5, 6, FIELD_KIND::UINT},
    {TYPE::QNTY, 11, 8, FIELD_KIND::UINT},
    {TYPE::STCK, 19, 8, FIELD_KIND::ASCII},
    {TYPE::CROSS_PRICE, 27, 4, FIELD_KIND::PRICE},
    {TYPE::MATCH_NO, 31, 8, FIELD_KIND::UINT},
    {TYPE::CROSS_TYPE, 39, 1, FIELD_KIND::CHAR},
}};

constexpr std::array<FieldSpec, 5> kBrokenTradeFields{{
    {TYPE::MSGTYPE, 0, 1, FIELD_KIND::CHAR},
    {TYPE::STCK_LOC, 1, 2, FIELD_KIND::UINT},
    {TYPE::TRCK_NO, 3, 2, FIELD_KIND::UINT},
    {TYPE::TIMESTAMP, 5, 6, FIELD_KIND::UINT},
    {TYPE::MATCH_NO, 11, 8, FIELD_KIND::UINT},
}};

[[nodiscard]] inline std::uint64_t readBe(const std::uint8_t* p, std::size_t n) {
    std::uint64_t v = 0;
    for (std::size_t i = 0; i < n; ++i) {
        v = (v << 8U) | p[i];
    }
    return v;
}

template <std::size_t N>
inline void parseWithSpec(const std::uint8_t* msg,
                          const std::array<FieldSpec, N>& spec,
                          Field* outFields,
                          std::size_t& fieldCount) {
    for (std::size_t i = 0; i < N; ++i) {
        const FieldSpec& f = spec[i];
        Field& out = outFields[i];
        out.type = f.type;
        out.kind = f.kind;
        out.u64 = 0;
        out.ascii_len = 0;

        switch (f.kind) {
            case FIELD_KIND::UINT:
            case FIELD_KIND::PRICE:
                out.u64 = readBe(msg + f.offset, f.length);
                break;
            case FIELD_KIND::CHAR:
                out.setChar(static_cast<char>(msg[f.offset]));
                break;
            case FIELD_KIND::ASCII:
                out.setAscii(reinterpret_cast<const char*>(msg + f.offset), f.length);
                break;
        }
    }
    fieldCount = N;
}

}  // namespace

std::size_t ITCH::messageLength(char type) {
    switch (type) {
        case 'A': return 36;
        case 'F': return 40;
        case 'E': return 31;
        case 'C': return 36;
        case 'X': return 23;
        case 'D': return 18;
        case 'U': return 35;
        case 'P': return 44;
        case 'Q': return 40;
        case 'B': return 19;
        default: return 0;
    }
}

void ITCH::parseMsg(const std::uint8_t* msg,
                    Field* outFields,
                    std::size_t& fieldCount) const {
    fieldCount = 0;
    if (APEX_UNLIKELY(msg == nullptr || outFields == nullptr)) {
        return;
    }

    const char type = static_cast<char>(msg[0]);
    switch (type) {
        case 'A':
            parseWithSpec(msg, kAddOrderFields, outFields, fieldCount);
            return;
        case 'F':
            parseWithSpec(msg, kAddOrderWithMpidFields, outFields, fieldCount);
            return;
        case 'E':
            parseWithSpec(msg, kOrderExecutedFields, outFields, fieldCount);
            return;
        case 'C':
            parseWithSpec(msg, kOrderExecutedPriceFields, outFields, fieldCount);
            return;
        case 'X':
            parseWithSpec(msg, kOrderCancelFields, outFields, fieldCount);
            return;
        case 'D':
            parseWithSpec(msg, kOrderDeleteFields, outFields, fieldCount);
            return;
        case 'U':
            parseWithSpec(msg, kOrderReplaceFields, outFields, fieldCount);
            return;
        case 'P':
            parseWithSpec(msg, kTradeFields, outFields, fieldCount);
            return;
        case 'Q':
            parseWithSpec(msg, kCrossTradeFields, outFields, fieldCount);
            return;
        case 'B':
            parseWithSpec(msg, kBrokenTradeFields, outFields, fieldCount);
            return;
        default:
            return;
    }
}

void ITCH::processBuffer(const std::uint8_t* buffer,
                         std::size_t length,
                         const MessageCallback& onMessage) {
    if (APEX_UNLIKELY(buffer == nullptr || length == 0 || !onMessage)) {
        return;
    }

    std::size_t cursor = 0;

    auto emit = [&](const std::uint8_t* msg) {
        Field fields[kMaxParsedFields];
        std::size_t fieldCount = 0;
        parseMsg(msg, fields, fieldCount);
        if (fieldCount > 0) {
            onMessage(fields, fieldCount);
        }
    };

    if (partial_len_ > 0) {
        if (expected_len_ == 0) {
            expected_len_ = messageLength(static_cast<char>(partial_msg_[0]));
            if (APEX_UNLIKELY(expected_len_ == 0)) {
                partial_len_ = 0;
                return;
            }
        }

        const std::size_t needed = expected_len_ - partial_len_;
        const std::size_t copyLen = std::min(needed, length);
        std::memcpy(partial_msg_.data() + partial_len_, buffer, copyLen);
        partial_len_ += copyLen;
        cursor += copyLen;

        if (partial_len_ < expected_len_) {
            return;
        }

        emit(partial_msg_.data());
        partial_len_ = 0;
        expected_len_ = 0;
    }

    while (cursor < length) {
        const char type = static_cast<char>(buffer[cursor]);
        const std::size_t msgLen = messageLength(type);

        if (APEX_UNLIKELY(msgLen == 0)) {
            ++cursor;
            continue;
        }

        const std::size_t remaining = length - cursor;
        if (APEX_LIKELY(remaining >= msgLen)) {
            emit(buffer + cursor);
            cursor += msgLen;
            continue;
        }

        std::memcpy(partial_msg_.data(), buffer + cursor, remaining);
        partial_len_ = remaining;
        expected_len_ = msgLen;
        break;
    }
}
