#pragma once
#include <cstdint>
#include <optional>
#include <array>

namespace Protocol {

static constexpr uint16_t max_payload_size = 1024;

struct Frame {
    std::array<uint8_t, 5 + max_payload_size> bytes;
    uint16_t len;
};


enum class Type : uint8_t {
    PING=0x01,
    PONG=0x02,
    RADIO_SEND=0x10,
    RADIO_SEND_QUEUED=0x11,
    RADIO_RECEIVED=0x12,
    AT_CMD=0x20,
    AT_RESP=0x21,
    AT_CMD_FAILED=0x22,
};

struct Packet {
    Type    type;
    uint16_t len;
    uint8_t payload[max_payload_size];
};

struct Parser {
    enum class State { SOF, TYPE, LEN_LOW, LEN_HIGH, PAYLOAD, CHECKSUM } state = State::SOF;
    Packet  pending;
    uint16_t cursor = 0;
    bool    ready  = false;
};

void feed(Parser&, uint8_t byte); // push one byte into the state machine
std::optional<Packet> take(Parser&);

std::optional<Frame> encode(const Packet& packet);

} // namespace Protocol