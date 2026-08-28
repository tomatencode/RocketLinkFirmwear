#include <Arduino.h>

#include "blinkLed.hpp"
#include "HC12.hpp"

#define LED_TX_PIN PB6
#define LED_RX_PIN PB7
#define HC12_SET_PIN PA8


HC12 hc12(HC12_SET_PIN, PA10, PA9);

BlinkLed txLed(LED_TX_PIN, 30, 0.15);
BlinkLed rxLed(LED_RX_PIN, 30, 1.0);

void setup() {
  pinMode(LED_TX_PIN, OUTPUT);
  pinMode(LED_RX_PIN, OUTPUT);
  pinMode(HC12_SET_PIN, OUTPUT);
  digitalWrite(HC12_SET_PIN, HIGH);

  Serial.begin(9600);
  hc12.begin();
  
  uint32_t start = millis();
  while (!Serial && millis() - start < 3000);

  hc12.sendATCommand("AT+DEFAULT");
  Serial.println("RocketLink ready.");
}

void loop() {
  txLed.update();
  rxLed.update();

  // PC -> HC12
  while (Serial.available()) {
    char c = static_cast<char>(Serial.read());
    hc12.send(&c);
    txLed.flash();
  }

  // HC12 -> PC
  while (hc12.receive().size() > 0) {
    std::string data = hc12.receive();
    for (char c : data) {
      Serial.write(c);
      rxLed.flash();
    }
  }
}
