#ifndef ITCH_H
#define ITCH_H

#include <array>
#include <cstddef>
#include <cstdint>
#include "Parser.h"

class ITCH final : public Parser {
public:
    static constexpr std::size_t kMaxMessageLength = 44;

    ITCH() = default;
    ~ITCH() override = default;

    void processBuffer(const std::uint8_t* buffer,
                       std::size_t length,
                       const MessageCallback& onMessage) override;

    void parseMsg(const std::uint8_t* msg,
                  Field* outFields,
                  std::size_t& fieldCount) const override;

    static std::size_t messageLength(char type);

private:
    std::array<std::uint8_t, kMaxMessageLength> partial_msg_{};
    std::size_t partial_len_{0};
    std::size_t expected_len_{0};
};

#endif
