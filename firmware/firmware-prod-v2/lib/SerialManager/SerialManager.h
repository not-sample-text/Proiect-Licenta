#pragma once

#include <Arduino.h>

class SerialManager {
public:
    static void begin();
    static void check();

private:
    static void handleConfigRead();
    static void handleConfigWrite();
    
    static char serialBuffer[64];
    static uint8_t bufferIndex;
};
