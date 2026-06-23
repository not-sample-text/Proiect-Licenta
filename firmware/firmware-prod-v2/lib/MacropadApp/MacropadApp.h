#pragma once

#include <Arduino.h>

class MacropadApp {
public:
    static void begin();
    static void run();

    static uint8_t getCurrentLayer() { return currentLayer; }
    static bool isBleMode() { return bleModeActive; }
    static const char* getLastKeyLabel() { return lastKeyLabel; }
    static void resetActivityTimer() { lastActivityMs = millis(); }

private:
    static void processEvents();
    static void checkInactivityTimeout(uint32_t now);

    static uint8_t currentLayer;
    static bool bleModeActive;
    static bool lastBleSwitchState;
    static uint32_t lastActivityMs;
    static const char* lastKeyLabel;

    static constexpr uint32_t DEEP_SLEEP_TIMEOUT_MS = 300000;  // Bumped to 5 minutes
};
