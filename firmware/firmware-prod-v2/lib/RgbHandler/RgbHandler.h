#pragma once

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

enum class BleLedState : uint8_t {
    kDisconnected,
    kPairing,
    kJustConnected,
    kConnectedTracking
};

class RgbHandler {
public:
    static void begin();
    static void run();
    
    // State trigger hooks called by network callbacks
    static void setBleState(BleLedState state);

private:
    static void handleStatusLed(uint32_t now);

    static Adafruit_NeoPixel statusLed;
    static BleLedState currentBleState;
    static uint32_t stateTransitionTimer;
    static bool blinkToggle;
    static uint32_t lastBlinkChangeMs;
};
