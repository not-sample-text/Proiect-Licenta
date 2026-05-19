#pragma once

#include <Arduino.h>

/**
 * @brief Handles the EC11 rotary encoder using interrupts for rotation
 *        and non-blocking polling for the button.
 */
class EncoderHandler {
public:
    /**
     * @brief Configures GPIOs and attaches interrupts.
     */
    static void begin();

    /**
     * @brief Processes accumulated rotation and button state. Polled in main loop.
     */
    static void run();

private:
    static void IRAM_ATTR handleISR();

    static volatile int8_t encoderDelta;
    static uint8_t lastState;
    
    // Button state
    static bool lastButtonState;
    static uint32_t lastButtonDebounce;
    static constexpr uint8_t BUTTON_DEBOUNCE_MS = 10;
};
