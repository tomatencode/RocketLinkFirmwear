#pragma once
#include <cstdint>
#include <string>
#include <span>


class HC12 {
public:
    HC12(int setPin,
         int rxPin,
         int txPin,
         int baudRate = 9600,
         std::function<void()> onSendCallback = nullptr,
         std::function<void()> onReceiveCallback = nullptr
        );

    void begin();
    
    void send(std::span<const uint8_t> data);
    bool available();
    uint8_t read();

    std::string sendATCommand(const char* command, uint32_t timeout_ms = 200);
private:
    int _setPin;
    int _rxPin;
    int _txPin;

    int _baudRate;

    HardwareSerial _serial;

    std::function<void()> _onSendCallback;
    std::function<void()> _onReceiveCallback;
};
