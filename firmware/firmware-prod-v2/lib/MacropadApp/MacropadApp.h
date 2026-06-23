#pragma once

#include <Arduino.h>

class MacropadApp {
public:
    static void begin();
    static void run();

    static uint8_t getCurrentLayer() { return currentLayer; }
    static bool isBleMode() { return bleModeActive; }
    // Now returns the safely buffered string
    static const char* getLastKeyLabel() { return currentLabel; }
    static void resetActivityTimer() { lastActivityMs = millis(); }

private:
    static void processEvents();
    static void checkInactivityTimeout(uint32_t now);

    static uint8_t currentLayer;
    static bool bleModeActive;
    static bool lastBleSwitchState;
    static uint32_t lastActivityMs;
    
    // Dedicated buffer to prevent dangling pointer / RAM overwrite issues
    static char currentLabel[32]; 

    static constexpr uint32_t DEEP_SLEEP_TIMEOUT_MS = 300000;  // 5 minutes
};
