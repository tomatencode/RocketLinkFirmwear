#include "bridge.hpp"
#include <cstring>

void Bridge::poll() {
    while (_usbSerial.available()) {
        Protocol::feed(_parser, _usbSerial.read());
        auto packetOpt = Protocol::take(_parser);
        if (packetOpt) handlePacket(*packetOpt);
    }

    while (_hc12.available()) {
        Protocol::Packet fwd;
        fwd.type = Protocol::Type::DATA;
        fwd.len = 0;
        do {
            fwd.payload[fwd.len++] = _hc12.read();
        } while (_hc12.available() && fwd.len < 255);
        auto frame = Protocol::encode(fwd);
        _usbSerial.write(frame.bytes.data(), frame.len);
    }
}

void Bridge::handlePacket(const Protocol::Packet& packet) {
    switch (packet.type) {
        case Protocol::Type::PING:
            {
                Protocol::Packet response;

                response.type = Protocol::Type::PONG;
                const char* magic = "RocketLink";
                response.len = static_cast<uint8_t>(strlen(magic));
                memcpy(response.payload, magic, response.len);

                auto frame = Protocol::encode(response);
                _usbSerial.write(frame.bytes.data(), frame.len);
            }
            break;
        case Protocol::Type::DATA:
            _hc12.send({packet.payload, packet.len});
            break;
        case Protocol::Type::AT_CMD:
            {
                std::string command(reinterpret_cast<const char*>(packet.payload), packet.len);
                std::string response = _hc12.sendATCommand(command.c_str());

                if (response.size() > 255) {
                    response.resize(255);
                }
                
                Protocol::Packet responsePacket;
                responsePacket.type = Protocol::Type::AT_RESP;
                responsePacket.len = static_cast<uint8_t>(response.size());
                std::copy(response.begin(), response.end(), responsePacket.payload);
                
                auto frame = Protocol::encode(responsePacket);
                _usbSerial.write(frame.bytes.data(), frame.len);
            }
            break;
        default:
            // Unknown packet type ignore
            break;
    }
}