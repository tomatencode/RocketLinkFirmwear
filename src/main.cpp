#include <Arduino.h>

#include "blinkLed.hpp"
#include "hc12.hpp"
#include "bridge.hpp"

#define LED_TX_PIN PB6
#define LED_RX_PIN PB7
#define HC12_SET_PIN PA8


BlinkLed txLed(LED_TX_PIN, 30, 0.15);
BlinkLed rxLed(LED_RX_PIN, 30, 1.0);

HC12 hc12(HC12_SET_PIN, PA10, PA9, 9600,
    []() { txLed.flash(); }, // onSendCallback
    []() { rxLed.flash(); }  // onReceiveCallback
);

Bridge bridge(hc12, Serial);

void setup() {
  Serial.begin(9600);
  hc12.begin();
  txLed.begin();
  rxLed.begin();
  
  uint32_t start = millis();
  while (!Serial && millis() - start < 3000);

  hc12.sendATCommand("AT+DEFAULT");
}

void loop() {
  txLed.update();
  rxLed.update();
  bridge.poll();
}
