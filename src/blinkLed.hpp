#pragma once
#include <cstdint>

class BlinkLed {
public:
    BlinkLed(int pin, int onDuration_ms, float brightness);

    void begin();

    void update();
    void flash();
private:
    int _pin;
    int _onDuration_ms;
    float _brightness;
    uint32_t _flashtime = 0;
};