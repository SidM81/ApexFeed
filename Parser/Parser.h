#ifndef PARSER_H
#define PARSER_H

#include <bits/stdc++.h>
#include "ITCHTypes.h"
using namespace std;

class Parser {
public:
    Parser() = default;
    virtual ~Parser() = default;

    virtual void processBuffer(const uint8_t *buffer, size_t length, std::function<void(const Field*, size_t)> onMessage) = 0;
    virtual void parseMsg(const uint8_t* msg, Field* outFields, size_t& fieldCount) = 0;

protected:
    static constexpr int MAX_MSG_TYPES = 256;
    static constexpr int NUM_TYPES = 20;
    array<vector<uint8_t>, MAX_MSG_TYPES> msgFields;
    array<array<pair<int,int>, NUM_TYPES>, MAX_MSG_TYPES> offsetTable;
};

#endif