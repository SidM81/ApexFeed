#ifndef ITCH_H
#define ITCH_H

#include "Parser.h"
#include "ITCHTypes.h"

class ITCH : public Parser {
    public:
        ITCH();
        ~ITCH();

        void processBuffer(const uint8_t *buffer, size_t length, std::function<void(const Field*, size_t)> onMessage) override;
        void parseMsg(const uint8_t* msg, Field* outFields, size_t& fieldCount) override;

    private:
        vector<uint8_t> incompleteMsgBuffer;
};

constexpr size_t idx(TYPE t) {
    return static_cast<size_t>(t);
}

#endif