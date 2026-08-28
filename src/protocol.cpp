#include "protocol.hpp"

uint8_t crc8(const Protocol::Packet& packet) {
    uint8_t crc = 0;
    crc ^= static_cast<uint8_t>(packet.type);
    crc ^= packet.len;
    for (uint8_t i = 0; i < packet.len; ++i) {
        crc ^= packet.payload[i];
    }
    return crc;
}

void Protocol::feed(Parser& parser, uint8_t byte) {
    switch (parser.state) {
        case Parser::State::SOF:
            if (byte == 0xAA) {
                parser.state = Parser::State::TYPE;
                parser.cursor = 0;
                parser.ready = false;
            }
            break;
        case Parser::State::TYPE:
            switch (byte) {
                case static_cast<uint8_t>(Type::PING):
                case static_cast<uint8_t>(Type::PONG):
                case static_cast<uint8_t>(Type::DATA):
                case static_cast<uint8_t>(Type::AT_CMD):
                case static_cast<uint8_t>(Type::AT_RESP):
                    parser.pending.type = static_cast<Type>(byte);
                    parser.state = Parser::State::LEN;
                    break;
                default:
                    parser.state = Parser::State::SOF;
                    break;
            }
            break;
        case Parser::State::LEN:
            parser.pending.len = byte;
            if (byte > 0) {
                parser.state = Parser::State::PAYLOAD;
            } else {
                parser.state = Parser::State::CHECKSUM;
            }
            break;
        case Parser::State::PAYLOAD:
            parser.pending.payload[parser.cursor++] = byte;
            if (parser.cursor >= parser.pending.len) {
                parser.state = Parser::State::CHECKSUM;
            }
            break;
        case Parser::State::CHECKSUM:
            if (byte == crc8(parser.pending)) {
                parser.ready = true;
            }
            parser.state = Parser::State::SOF; // Reset for next packet
            break;
    }
}

std::optional<Protocol::Packet> Protocol::take(Parser& parser) {
    if (parser.ready) {
        parser.ready = false;
        return parser.pending;
    }
    return std::nullopt;
}


Protocol::Frame Protocol::encode(const Packet& packet) {
    Frame frame;
    frame.bytes[0] = 0xAA; // Start of Frame
    frame.bytes[1] = static_cast<uint8_t>(packet.type);
    frame.bytes[2] = packet.len;
    for (uint8_t i = 0; i < packet.len; ++i) {
        frame.bytes[3 + i] = packet.payload[i];
    }
    frame.bytes[3 + packet.len] = crc8(packet);
    frame.len = 4 + packet.len; // SOF + TYPE + LEN + PAYLOAD + CRC
    return frame;
}