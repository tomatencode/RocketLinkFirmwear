#pragma once
#include <Arduino.h>
#include <string>


class HC12 {
public:
    HC12(int setPin, int rxPin, int txPin);
    void begin();
    
    void send(const char* data);
    [[nodiscard]] std::string receive();

    std::string sendATCommand(const char* command, uint32_t timeout_ms = 200);
private:
    int _setPin;
    int _rxPin;
    int _txPin;
    HardwareSerial _serial;
};
