#pragma once
#include <Arduino.h>

class BlinkLed {
public:
    BlinkLed(int pin, int onDuration_ms, double brightness);
    void update();
    void flash();
private:
    int _pin;
    int _onDuration_ms;
    double _brightness;
    uint32_t _ledOff = 0;
};