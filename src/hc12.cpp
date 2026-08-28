#include <Arduino.h>
#include "hc12.hpp"

HC12::HC12(int setPin, int rxPin, int txPin, int baudRate, std::function<void()> onSendCallback, std::function<void()> onReceiveCallback)
    : _setPin(setPin), _rxPin(rxPin), _txPin(txPin), _baudRate(baudRate), _onSendCallback(onSendCallback), _onReceiveCallback(onReceiveCallback), _serial(_rxPin, _txPin) {
    pinMode(_setPin, OUTPUT);
    digitalWrite(_setPin, HIGH); // default to normal mode
}

void HC12::begin() {
    _serial.begin(_baudRate);
}

void HC12::send(std::span<const uint8_t> data) {
    if (_onSendCallback && data.size() > 0) {
        _onSendCallback();
    }
    for (auto byte : data) {
        _serial.write(byte);
    }
}

bool HC12::available() {
    return _serial.available() > 0;
}

uint8_t HC12::read() {
    if (available() && _onReceiveCallback) {
        _onReceiveCallback();
    }
    return _serial.read();
}

std::string HC12::sendATCommand(const char* command, uint32_t timeout_ms) {
    digitalWrite(_setPin, LOW); // enter AT mode
    delay(100); // wait for the module to enter AT mode

    // Clear any existing data in the serial buffer
    while (_serial.available()) _serial.read();

    _serial.print(command);
    _serial.print("\r\n");

    uint32_t start = millis();
    std::string response;
    while (millis() - start < timeout_ms) {
        if (_serial.available()) {
            response += static_cast<char>(_serial.read());
        }
    }

    digitalWrite(_setPin, HIGH); // exit AT mode
    delay(100); // wait for the module to exit AT mode

    return response;
}