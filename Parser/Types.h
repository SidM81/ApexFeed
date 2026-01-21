#ifndef TYPES_H
#define TYPES_H

enum class TYPE : size_t {
    MSGTYPE = 0,
    STCK_LOC = 1,
    TRCK_NO  = 2,
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

enum class FIELD_KIND {
    UINT,       // binary integer
    ASCII,      // fixed-width text
    CHAR,       // single character
    PRICE       // scaled integer
};

struct FieldMeta {
    FIELD_KIND kind;
};

static constexpr FieldMeta FIELD_META[] = {
    {FIELD_KIND::CHAR},   // MSGTYPE = 0
    {FIELD_KIND::UINT},   // STCK_LOC
    {FIELD_KIND::UINT},   // TRCK_NO
    {FIELD_KIND::UINT},   // TIMESTAMP
    {FIELD_KIND::UINT},   // ORDR_REF_NO
    {FIELD_KIND::UINT},   // NEW_ORDR_REF_NO
    {FIELD_KIND::UINT},   // BUY_ORDR_REF_NO
    {FIELD_KIND::UINT},   // SELL_ORDR_REF_NO
    {FIELD_KIND::CHAR},   // SIDE
    {FIELD_KIND::UINT},   // QNTY
    {FIELD_KIND::ASCII},  // STCK
    {FIELD_KIND::PRICE},  // PRICE
    {FIELD_KIND::PRICE},  // CROSS_PRICE
    {FIELD_KIND::ASCII},  // CROSS_TYPE (could be CHAR/ASCII depending on spec)
    {FIELD_KIND::UINT},   // ATTRBTN
    {FIELD_KIND::UINT},   // EXEC_SHARES
    {FIELD_KIND::UINT},   // MATCH_NO
    {FIELD_KIND::CHAR},   // PRNTABLE
    {FIELD_KIND::PRICE},  // EXEC_PRICE
    {FIELD_KIND::UINT},   // CNCLD_SHARES
    {FIELD_KIND::ASCII}   // NONE (won’t really be used)
};



struct Field {
    TYPE type;
    uint64_t u64;
    std::string s;
    bool is_string;
};


#endif
