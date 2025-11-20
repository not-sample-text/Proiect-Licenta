#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>

class BleHandler {
public:
    BleHandler();
    void begin();
    void sendKey(uint8_t keycode);
    bool isConnected();
};
