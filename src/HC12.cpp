#include <Arduino.h>
#include "HC12.hpp"

HC12::HC12(int setPin, int rxPin, int txPin)
    : _setPin(setPin), _rxPin(rxPin), _txPin(txPin), _serial(_rxPin, _txPin) {
    pinMode(_setPin, OUTPUT);
    digitalWrite(_setPin, HIGH); // default to normal mode
}

void HC12::begin() {
    _serial.begin(9600);
}

void HC12::send(const char* data) {
    _serial.print(data);
}   

std::string HC12::receive() {
    std::string result;
    while (_serial.available()) {
        result += static_cast<char>(_serial.read());
    }
    return result;
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