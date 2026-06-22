#pragma once

#include <Arduino.h>

class EncoderHandler {
public:
    static void begin();
    static void run();
    static bool isSwitchPressed() { return digitalRead(1) == LOW; }
    static uint8_t getLastStateCode() { return encoderLastState; }
    static int8_t getAccumulatedSteps() { return encoderAccumulatedSteps; }
    
    // Fixes the OledHandler compilation error by exposing the interrupt flag cleanly
    static bool isInterruptPending(); 

private:
    static uint8_t readRawState();

    static uint8_t encoderLastState;
    static int8_t encoderAccumulatedSteps;

    // Button tracking parameters
    static bool lastButtonState;
    static uint32_t lastButtonDebounce;
    static uint32_t buttonPressStartTime;
    static bool shutdownTriggered;

    static constexpr uint8_t BUTTON_DEBOUNCE_MS = 10;
    static constexpr uint32_t SHUTDOWN_HOLD_MS = 2000;
};
