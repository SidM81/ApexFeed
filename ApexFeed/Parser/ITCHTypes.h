#ifndef ITCHTYPES_H
#define ITCHTYPES_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>

enum class TYPE : std::uint8_t {
    MSGTYPE = 0,
    STCK_LOC = 1,
    TRCK_NO = 2,
    TIMESTAMP = 3,
    ORDR_REF_NO = 4,
    NEW_ORDR_REF_NO = 5,
    BUY_ORDR_REF_NO = 6,
    SELL_ORDR_REF_NO = 7,
    SIDE = 8,
    QNTY = 9,
    STCK = 10,
    PRICE = 11,
    CROSS_PRICE = 12,
    CROSS_TYPE = 13,
    ATTRBTN = 14,
    EXEC_SHARES = 15,
    MATCH_NO = 16,
    PRNTABLE = 17,
    EXEC_PRICE = 18,
    CNCLD_SHARES = 19,
    NONE = 20
};

enum class FIELD_KIND : std::uint8_t {
    UINT,
    ASCII,
    CHAR,
    PRICE
};

struct Field {
    TYPE type{TYPE::NONE};
    FIELD_KIND kind{FIELD_KIND::UINT};
    std::uint64_t u64{0};
    std::uint8_t ascii_len{0};
    std::array<char, 16> ascii{};

    [[nodiscard]] inline std::string_view asStringView() const {
        return std::string_view(ascii.data(), ascii_len);
    }

    inline void setChar(char c) {
        ascii_len = 1;
        ascii[0] = c;
        for (std::size_t i = 1; i < ascii.size(); ++i) {
            ascii[i] = '\0';
        }
    }

    inline void setAscii(const char* src, std::size_t len) {
        const std::size_t clipped = len > ascii.size() ? ascii.size() : len;
        std::memcpy(ascii.data(), src, clipped);
        ascii_len = static_cast<std::uint8_t>(clipped);
        if (clipped < ascii.size()) {
            std::memset(ascii.data() + clipped, 0, ascii.size() - clipped);
        }
    }
};

constexpr std::size_t idx(TYPE t) {
    return static_cast<std::size_t>(t);
}

constexpr std::size_t kMaxParsedFields = 16;

#endif
