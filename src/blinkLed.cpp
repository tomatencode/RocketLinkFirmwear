#include "blinkLed.hpp"

BlinkLed::BlinkLed(int pin, int onDuration_ms, double brightness)
    : _pin(pin), _onDuration_ms(onDuration_ms), _brightness(brightness) {
    pinMode(_pin, OUTPUT);
}

void BlinkLed::update() {
    if (millis() >= _ledOff) {
        analogWrite(_pin, 0);
    }
}

void BlinkLed::flash() {
    analogWrite(_pin, (int)(255 * _brightness));
    _ledOff = millis() + _onDuration_ms;
}