#ifndef PARSER_H
#define PARSER_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include "ITCHTypes.h"

class Parser {
public:
    using MessageCallback = std::function<void(const Field*, std::size_t)>;

    virtual ~Parser() = default;

    virtual void processBuffer(const std::uint8_t* buffer,
                               std::size_t length,
                               const MessageCallback& onMessage) = 0;

    virtual void parseMsg(const std::uint8_t* msg,
                          Field* outFields,
                          std::size_t& fieldCount) const = 0;
};

#endif
