#pragma once
#include "hc12.hpp"
#include "protocol.hpp"
#include <Arduino.h>

class Bridge {
public:
    explicit Bridge(HC12& hc12, Stream& usbSerial) : _hc12(hc12), _usbSerial(usbSerial) {}
    void poll();

private:
    HC12& _hc12;
    Stream& _usbSerial;
    Protocol::Parser _parser;

    void handlePacket(const Protocol::Packet&);
};