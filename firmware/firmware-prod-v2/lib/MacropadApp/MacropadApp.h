#pragma once

#include <Arduino.h>

class MacropadApp {
public:
    static void begin();
    static void run();

    static uint8_t getCurrentLayer() { return currentLayer; }
    static bool isBleMode() { return bleModeActive; }
    static char getLastKey() { return lastKey; } // Public getter for OledHandler

private:
    static void processEvents();
    static void checkInactivityTimeout(uint32_t now);

    static uint8_t currentLayer;
    static bool bleModeActive;
    static bool lastBleSwitchState;
    static uint32_t lastActivityMs;
    static char lastKey;

    static constexpr uint32_t LIGHT_SLEEP_TIMEOUT_MS = 10000; // 10s
    static constexpr uint32_t DEEP_SLEEP_TIMEOUT_MS = 30000;  // 30s
};
