#include "bridge.hpp"

void Bridge::poll() {
    while (_usbSerial.available()) {
        Protocol::feed(_parser, _usbSerial.read());
        auto packetOpt = Protocol::take(_parser);
        if (packetOpt) handlePacket(*packetOpt);
    }

    while (_hc12.available()) {
        Protocol::Packet fwd;
        fwd.type = Protocol::Type::RADIO_RECEIVED;
        fwd.len = 0;
        do {
            auto byteOpt = _hc12.read();
            if (!byteOpt) break; // Should not happen
            fwd.payload[fwd.len++] = *byteOpt;

            // allow more time for the next byte to arrive, since the HC12 is slow
            // note this also blocks usb packages to be handled but we don't want that while receiving data anyway
            // since that would put the HC12 out of receive mode and cause data loss
            uint32_t start = millis();
            while (millis() - start < 3) {
                if (_hc12.available()) break;
            }

        } while (_hc12.available() && fwd.len < Protocol::max_payload_size);
        
        auto frame = Protocol::encode(fwd);
        if (frame) {
            _usbSerial.write(frame->bytes.data(), frame->len);
        }
    }

    if (_hc12.atDone()) {
        auto responseOpt = _hc12.takeATResponse();
        if (responseOpt) {
            auto& response = *responseOpt;
            if (response.size() > Protocol::max_payload_size) {
                response.resize(Protocol::max_payload_size);
            }
            
            Protocol::Packet responsePacket;
            responsePacket.type = Protocol::Type::AT_RESP;
            responsePacket.len = static_cast<uint16_t>(response.size());
            std::copy(response.begin(), response.end(), responsePacket.payload);
            
            auto frame = Protocol::encode(responsePacket);
            if (frame) {
                _usbSerial.write(frame->bytes.data(), frame->len);
            }
        }
    }
}

void Bridge::handlePacket(const Protocol::Packet& packet) {
    switch (packet.type) {
        case Protocol::Type::PING:
            {
                Protocol::Packet response;
                response.type = Protocol::Type::PONG;
                response.len = 0;

                auto frame = Protocol::encode(response);
                if (frame) {
                    _usbSerial.write(frame->bytes.data(), frame->len);
                }
            }
            break;
        case Protocol::Type::RADIO_SEND:
            {
                boolean success = _hc12.send({packet.payload, packet.len});

                Protocol::Packet responsePacket;
                responsePacket.type = success ? Protocol::Type::RADIO_SEND_QUEUED : Protocol::Type::RADIO_SEND_FAILED;
                responsePacket.len = 0;
                
                auto frame = Protocol::encode(responsePacket);
                if (frame) {
                    _usbSerial.write(frame->bytes.data(), frame->len);
                }
            }
            break;
        case Protocol::Type::AT_CMD:
            {
                std::string command(reinterpret_cast<const char*>(packet.payload), packet.len);
                bool success = _hc12.sendATCommand(command.c_str());
                if (!success) {
                    Protocol::Packet responsePacket;
                    responsePacket.type = Protocol::Type::AT_CMD_SEND_FAILED;
                    responsePacket.len = 0;

                    auto frame = Protocol::encode(responsePacket);
                    if (frame) {
                        _usbSerial.write(frame->bytes.data(), frame->len);
                    }
                }
            }
            break;
        default:
            // Unknown packet type ignore
            break;
    }
}