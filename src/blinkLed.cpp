#include <Arduino.h>
#include "blinkLed.hpp"

BlinkLed::BlinkLed(int pin, int onDuration_ms, uint8_t brightness)
    : _pin(pin), _onDuration_ms(onDuration_ms), _brightness(brightness) {
}

void BlinkLed::begin() {
    pinMode(_pin, OUTPUT);
    analogWrite(_pin, 0);
}

void BlinkLed::update() {
    if (millis() >= _flashtime + _onDuration_ms) {
        analogWrite(_pin, 0);
    }
}

void BlinkLed::flash() {
    analogWrite(_pin, _brightness);
    _flashtime = millis();
}