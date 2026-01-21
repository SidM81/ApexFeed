#ifndef ITCH_H
#define ITCH_H

#include "Parser.h"
#include "Types.h"

class ITCH : public Parser {
    public:
        ITCH();
        ~ITCH();

        void processBuffer(const uint8_t *buffer, size_t length) override;
        vector<Field> parseMsg(vector<uint8_t> message) override;

    private:
        vector<uint8_t> incompleteMsgBuffer;
        vector<vector<Field>> parsedMessages;
};

constexpr size_t idx(TYPE t) {
    return static_cast<size_t>(t);
}

#endif