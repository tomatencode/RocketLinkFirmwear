#pragma once
#include <cstdint>
#include <optional>
#include <array>

namespace Protocol {

struct Frame {
    std::array<uint8_t, 259> bytes;
    uint16_t len;
};


enum class Type : uint8_t { PING=0x01, PONG=0x02, DATA=0x10, AT_CMD=0x20, AT_RESP=0x21 };

struct Packet {
    Type    type;
    uint8_t len;
    uint8_t payload[255];
};

struct Parser {
    enum class State { SOF, TYPE, LEN, PAYLOAD, CHECKSUM } state = State::SOF;
    Packet  pending;
    uint8_t cursor = 0;
    bool    ready  = false;
};

void feed(Parser&, uint8_t byte); // push one byte into the state machine
std::optional<Packet> take(Parser&);

Frame encode(const Packet& packet);

} // namespace Protocol