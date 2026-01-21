#include "ITCH.h"
#include "Types.h"


ITCH::ITCH() {

    msgFields['A'] = {idx(TYPE::MSGTYPE), idx(TYPE::STCK_LOC), idx(TYPE::TRCK_NO), idx(TYPE::TIMESTAMP), idx(TYPE::ORDR_REF_NO), idx(TYPE::SIDE), idx(TYPE::QNTY), idx(TYPE::STCK), idx(TYPE::PRICE)};
    msgFields['F'] = {idx(TYPE::MSGTYPE), idx(TYPE::STCK_LOC), idx(TYPE::TRCK_NO), idx(TYPE::TIMESTAMP), idx(TYPE::ORDR_REF_NO), idx(TYPE::SIDE), idx(TYPE::QNTY), idx(TYPE::STCK), idx(TYPE::PRICE), idx(TYPE::ATTRBTN)};
    msgFields['E'] = {idx(TYPE::MSGTYPE), idx(TYPE::STCK_LOC), idx(TYPE::TRCK_NO), idx(TYPE::TIMESTAMP), idx(TYPE::ORDR_REF_NO), idx(TYPE::EXEC_SHARES), idx(TYPE::MATCH_NO)};
    msgFields['C'] = {idx(TYPE::MSGTYPE), idx(TYPE::STCK_LOC), idx(TYPE::TRCK_NO), idx(TYPE::TIMESTAMP), idx(TYPE::ORDR_REF_NO), idx(TYPE::EXEC_SHARES), idx(TYPE::MATCH_NO), idx(TYPE::PRNTABLE), idx(TYPE::EXEC_PRICE)};
    msgFields['X'] = {idx(TYPE::MSGTYPE), idx(TYPE::STCK_LOC), idx(TYPE::TRCK_NO), idx(TYPE::TIMESTAMP), idx(TYPE::ORDR_REF_NO), idx(TYPE::CNCLD_SHARES)};
    msgFields['D'] = {idx(TYPE::MSGTYPE), idx(TYPE::STCK_LOC), idx(TYPE::TRCK_NO), idx(TYPE::TIMESTAMP), idx(TYPE::ORDR_REF_NO)};
    msgFields['U'] = {idx(TYPE::MSGTYPE), idx(TYPE::STCK_LOC), idx(TYPE::TRCK_NO), idx(TYPE::TIMESTAMP), idx(TYPE::ORDR_REF_NO), idx(TYPE::NEW_ORDR_REF_NO), idx(TYPE::QNTY), idx(TYPE::PRICE)};
    msgFields['P'] = {idx(TYPE::MSGTYPE), idx(TYPE::STCK_LOC), idx(TYPE::TRCK_NO), idx(TYPE::TIMESTAMP), idx(TYPE::ORDR_REF_NO), idx(TYPE::SIDE), idx(TYPE::QNTY), idx(TYPE::STCK), idx(TYPE::PRICE), idx(TYPE::MATCH_NO)};
    msgFields['Q'] = {idx(TYPE::MSGTYPE), idx(TYPE::STCK_LOC), idx(TYPE::TRCK_NO), idx(TYPE::TIMESTAMP), idx(TYPE::QNTY), idx(TYPE::STCK), idx(TYPE::CROSS_PRICE), idx(TYPE::MATCH_NO), idx(TYPE::CROSS_TYPE)};
    msgFields['B'] = {idx(TYPE::MSGTYPE), idx(TYPE::STCK_LOC), idx(TYPE::TRCK_NO), idx(TYPE::TIMESTAMP), idx(TYPE::MATCH_NO)};


    offsetTable['A'][idx(TYPE::MSGTYPE)] = {0,1};
    offsetTable['A'][idx(TYPE::STCK_LOC)] = {1,2};
    offsetTable['A'][idx(TYPE::TRCK_NO)] = {3,2};
    offsetTable['A'][idx(TYPE::TIMESTAMP)] = {5,6};
    offsetTable['A'][idx(TYPE::ORDR_REF_NO)] = {11,8};
    offsetTable['A'][idx(TYPE::SIDE)] = {19,1};
    offsetTable['A'][idx(TYPE::QNTY)] = {20,4};
    offsetTable['A'][idx(TYPE::STCK)] = {24,8};
    offsetTable['A'][idx(TYPE::PRICE)] = {32,4};

    offsetTable['F'][idx(TYPE::MSGTYPE)] = {0,1};
    offsetTable['F'][idx(TYPE::STCK_LOC)] = {1,2};
    offsetTable['F'][idx(TYPE::TRCK_NO)] = {3,2};
    offsetTable['F'][idx(TYPE::TIMESTAMP)] = {5,6};
    offsetTable['F'][idx(TYPE::ORDR_REF_NO)] = {11,8};
    offsetTable['F'][idx(TYPE::SIDE)] = {19,1};
    offsetTable['F'][idx(TYPE::QNTY)] = {20,4};
    offsetTable['F'][idx(TYPE::STCK)] = {24,8};
    offsetTable['F'][idx(TYPE::PRICE)] = {32,4};
    offsetTable['F'][idx(TYPE::ATTRBTN)] = {36,4};

    offsetTable['E'][idx(TYPE::MSGTYPE)] = {0,1};
    offsetTable['E'][idx(TYPE::STCK_LOC)] = {1,2};
    offsetTable['E'][idx(TYPE::TRCK_NO)] = {3,2};
    offsetTable['E'][idx(TYPE::TIMESTAMP)] = {5,6};
    offsetTable['E'][idx(TYPE::ORDR_REF_NO)] = {11,8};
    offsetTable['E'][idx(TYPE::EXEC_SHARES)] = {19,4};
    offsetTable['E'][idx(TYPE::MATCH_NO)] = {23,8};

    offsetTable['C'][idx(TYPE::MSGTYPE)] = {0,1};
    offsetTable['C'][idx(TYPE::STCK_LOC)] = {1,2};
    offsetTable['C'][idx(TYPE::TRCK_NO)] = {3,2};
    offsetTable['C'][idx(TYPE::TIMESTAMP)] = {5,6};
    offsetTable['C'][idx(TYPE::ORDR_REF_NO)] = {11,8};
    offsetTable['C'][idx(TYPE::EXEC_SHARES)] = {19,4};
    offsetTable['C'][idx(TYPE::MATCH_NO)] = {23,8};
    offsetTable['C'][idx(TYPE::PRNTABLE)] = {31,1};
    offsetTable['C'][idx(TYPE::EXEC_PRICE)] = {32,4};

    offsetTable['X'][idx(TYPE::MSGTYPE)] = {0,1};
    offsetTable['X'][idx(TYPE::STCK_LOC)] = {1,2};
    offsetTable['X'][idx(TYPE::TRCK_NO)] = {3,2};
    offsetTable['X'][idx(TYPE::TIMESTAMP)] = {5,6};
    offsetTable['X'][idx(TYPE::ORDR_REF_NO)] = {11,8};
    offsetTable['X'][idx(TYPE::CNCLD_SHARES)] = {19,4};

    offsetTable['D'][idx(TYPE::MSGTYPE)] = {0,1};
    offsetTable['D'][idx(TYPE::STCK_LOC)] = {1,2};
    offsetTable['D'][idx(TYPE::TRCK_NO)] = {3,2};
    offsetTable['D'][idx(TYPE::TIMESTAMP)] = {5,6};
    offsetTable['D'][idx(TYPE::ORDR_REF_NO)] = {11,8};

    offsetTable['U'][idx(TYPE::MSGTYPE)] = {0,1};
    offsetTable['U'][idx(TYPE::STCK_LOC)] = {1,2};
    offsetTable['U'][idx(TYPE::TRCK_NO)] = {3,2};
    offsetTable['U'][idx(TYPE::TIMESTAMP)] = {5,6};
    offsetTable['U'][idx(TYPE::ORDR_REF_NO)] = {11,8};
    offsetTable['U'][idx(TYPE::NEW_ORDR_REF_NO)] = {19,8};
    offsetTable['U'][idx(TYPE::QNTY)] = {27,4};
    offsetTable['U'][idx(TYPE::PRICE)] = {31,4};

    offsetTable['P'][idx(TYPE::MSGTYPE)] = {0,1};
    offsetTable['P'][idx(TYPE::STCK_LOC)] = {1,2};
    offsetTable['P'][idx(TYPE::TRCK_NO)] = {3,2};
    offsetTable['P'][idx(TYPE::TIMESTAMP)] = {5,6};
    offsetTable['P'][idx(TYPE::ORDR_REF_NO)] = {11,8};
    offsetTable['P'][idx(TYPE::SIDE)] = {19,1};
    offsetTable['P'][idx(TYPE::QNTY)] = {20,4};
    offsetTable['P'][idx(TYPE::STCK)] = {24,8};
    offsetTable['P'][idx(TYPE::PRICE)] = {32,4};
    offsetTable['P'][idx(TYPE::MATCH_NO)] = {36,8};

    offsetTable['Q'][idx(TYPE::MSGTYPE)] = {0,1};
    offsetTable['Q'][idx(TYPE::STCK_LOC)] = {1,2};
    offsetTable['Q'][idx(TYPE::TRCK_NO)] = {3,2};
    offsetTable['Q'][idx(TYPE::TIMESTAMP)] = {5,6};
    offsetTable['Q'][idx(TYPE::QNTY)] = {11,8};
    offsetTable['Q'][idx(TYPE::STCK)] = {19,8};
    offsetTable['Q'][idx(TYPE::CROSS_PRICE)] = {27,4};
    offsetTable['Q'][idx(TYPE::MATCH_NO)] = {31,8};
    offsetTable['Q'][idx(TYPE::CROSS_TYPE)] = {39,1};

    offsetTable['B'][idx(TYPE::MSGTYPE)] = {0,1};
    offsetTable['B'][idx(TYPE::STCK_LOC)] = {1,2};
    offsetTable['B'][idx(TYPE::TRCK_NO)] = {3,2};
    offsetTable['B'][idx(TYPE::TIMESTAMP)] = {5,6};
    offsetTable['B'][idx(TYPE::MATCH_NO)] = {11,8};

}

ITCH::~ITCH() {
}

inline size_t itchMessageLength(char type) {
    switch (type) {
        case 'A': return 36;  // Add Order without MPID
        case 'F': return 40;  // Add Order with MPID
        case 'E': return 31;  // Execute Order
        case 'C': return 36;  // Execute with Price
        case 'X': return 23;  // Cancel Order
        case 'D': return 18;  // Delete Order
        case 'U': return 35;  // Order Replace
        case 'P': return 44;  // Trade
        case 'Q': return 40;  // Cross Trade
        case 'B': return 19;  // Broken Trade
        default:  return 0;
    }
}

inline size_t itchMessageNumFields(char type) {
    switch (type) {
        case 'A': return 9;  // Add Order without MPID
        case 'F': return 10;  // Add Order with MPID
        case 'E': return 6;  // Execute Order
        case 'C': return 9;  // Execute with Price
        case 'X': return 6;  // Cancel Order
        case 'D': return 5;  // Delete Order
        case 'U': return 8;  // Order Replace
        case 'P': return 10;  // Trade
        case 'Q': return 9;  // Cross Trade
        case 'B': return 5;  // Broken Trade
        default:  return 0;
    }
}

uint64_t read_be(const uint8_t* p, size_t size) {
    uint64_t v = 0;
    for (size_t i = 0; i < size; ++i) {
        v = (v << 8) | p[i];
    }
    return v;
}

static inline std::string u64_to_string(uint64_t v) {
    char buf[32]; 
    char* p = buf + sizeof(buf);
    *--p = '\0';

    do {
        *--p = char('0' + (v % 10));
        v /= 10;
    } while (v);

    return std::string(p);
}


vector<Field> ITCH::parseMsg(vector<uint8_t> message) {
    vector<Field> parsedFields;
    if (message.empty()) {
        return parsedFields;
    }

    char msgType = static_cast<char>(message[0]);

    for (size_t i = 0; i < itchMessageNumFields(msgType); ++i) {
        auto [offset, length] = offsetTable[msgType][msgFields[msgType][i]];
        if (length > 0) {

            TYPE field = static_cast<TYPE>(msgFields[msgType][i]);
            FIELD_KIND kind = FIELD_META[idx(field)].kind;

            switch (kind) {
                case FIELD_KIND::UINT: {
                    uint64_t value = read_be(&message[offset], length);
                    parsedFields.push_back({.type = field, .u64 = value, .is_string = false});
                    break;
                }
                case FIELD_KIND::CHAR: {
                    char c = static_cast<char>(message[offset]);
                    parsedFields.push_back({
                        .type = field,
                        .s = std::string(1, c),
                        .is_string = true
                    });
                    break;
                }
                case FIELD_KIND::ASCII: {
                    std::string s(
                        message.begin() + offset,
                        message.begin() + offset + length
                    );
                    parsedFields.push_back({.type = field, .s = s, .is_string = true});
                    break;
                }
                case FIELD_KIND::PRICE: {
                    uint64_t raw = read_be(&message[offset], length);
                    parsedFields.push_back({.type = field, .u64 = raw, .is_string = false});
                    break;
                }
            }
        } else {
            parsedFields.push_back({.type = TYPE::NONE,.s = "",.is_string = true}); // Empty field
        }
    }

    return parsedFields;
}


void ITCH::processBuffer(const uint8_t *buffer, size_t length) {
    size_t cursor = 0;

    if (incompleteMsgBuffer.size() > 0) {
        size_t needed = 0;
        if (incompleteMsgBuffer.size() >= 1) {
            char msgType = static_cast<char>(incompleteMsgBuffer[0]);
            needed = itchMessageLength(msgType);
        }
        if (needed == 0) {
            incompleteMsgBuffer.clear();
            return;
        }
        size_t toCopy = std::min(needed - incompleteMsgBuffer.size(), length);
        incompleteMsgBuffer.insert(incompleteMsgBuffer.end(), buffer, buffer + toCopy);
        cursor += toCopy;

        if (incompleteMsgBuffer.size() == needed) {
            parsedMessages.push_back(parseMsg(incompleteMsgBuffer));
            incompleteMsgBuffer.clear();
        } else {
            return;
        }
    }

    while (cursor < length) {

        char msgType = static_cast<char>(buffer[cursor]);
        size_t msgLength = itchMessageLength(msgType);

        if (msgLength == 0 || cursor + msgLength > length) {
            break; // Unknown message type
        }

        vector<uint8_t> message(buffer + cursor, buffer + cursor + msgLength);
        parsedMessages.push_back(parseMsg(message));

        cursor += msgLength;

    }
    incompleteMsgBuffer.insert(incompleteMsgBuffer.end(), buffer + cursor, buffer + length);
    return ;
}
