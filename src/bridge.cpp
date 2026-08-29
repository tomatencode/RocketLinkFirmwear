#include "bridge.hpp"
#include <cstring>

void Bridge::poll() {
    while (_usbSerial.available()) {
        Protocol::feed(_parser, _usbSerial.read());
        auto packetOpt = Protocol::take(_parser);
        if (packetOpt) handlePacket(*packetOpt);
    }

    while (_hc12.available()) {
        auto byteOpt = _hc12.read();
        if (byteOpt) {
            uint8_t byte = *byteOpt;
            if (_radioReceveBufferSize < _radioBuffer.max_size()) {
                _radioBuffer[_radioReceveBufferSize++] = byte;
            } else {
                // Buffer overflow, shift the buffer to make room for new data
                memmove(_radioBuffer.data(), _radioBuffer.data() + 1, _radioReceveBufferSize - 1);
                _radioBuffer[_radioReceveBufferSize - 1] = byte;
            }
        }
    }
}

void Bridge::sendPackedUsb(const Protocol::Packet& packet) {
    auto frame = Protocol::encode(packet);
    _usbSerial.write(frame.bytes.data(), frame.len);
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

                sendPackedUsb(response);
            }
            break;
        case Protocol::Type::SEND_RADIO_REQ:
            {
                _hc12.send({packet.payload, packet.len});
                sendPackedUsb({Protocol::Type::SEND_RADIO_ACK, 0, {}});
            }
            break;
        case Protocol::Type::RECEIVE_RADIO_REQ:
            {
                Protocol::Packet response;
                response.type = Protocol::Type::RECEIVE_RADIO_RESP;

                size_t bytesToSend = std::min(_radioReceveBufferSize, static_cast<size_t>(255));

                memcpy(response.payload, _radioBuffer.data(), bytesToSend);

                // Shift the buffer to remove the sent data
                if (bytesToSend < _radioReceveBufferSize) {
                    memmove(_radioBuffer.data(), _radioBuffer.data() + bytesToSend, _radioReceveBufferSize - bytesToSend);
                }
                _radioReceveBufferSize -= bytesToSend;

                // If we have less than 255 bytes, we send the currently incoming bytes if available
                if (bytesToSend < 255) {
                    while (_hc12.available() && bytesToSend < 255) {
                        auto byteOpt = _hc12.read();
                        if (byteOpt) {
                            uint8_t byte = *byteOpt;
                            response.payload[bytesToSend++] = byte;
                        }

                        // Wait for a short period to see if more bytes arrives
                        uint32_t startMillis = millis();
                        while (millis() - startMillis < 3) {
                            if (_hc12.available()) {
                                break; // New data is available, break the wait loop
                            }
                        }
                    }
                }

                response.len = static_cast<uint8_t>(bytesToSend);

                sendPackedUsb(response);
            }
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
                
                sendPackedUsb(responsePacket);
            }
            break;
        default:
            // Unknown packet type ignore
            break;
    }
}