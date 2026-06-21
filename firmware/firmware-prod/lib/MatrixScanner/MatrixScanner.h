#pragma once

#include <Arduino.h>

/**
 * @brief Handles non-blocking matrix scanning with software debounce.
 */
class MatrixScanner {
public:
    /**
     * @brief Configures GPIOs for the 3x4 matrix.
     */
    static void begin();

    /**
     * @brief Scans the matrix and enqueues events. Polled in main loop.
     */
    static void scan();

private:
    static bool currentState[4][3];
    static uint32_t lastDebounceTime[4][3];
    static constexpr uint8_t DEBOUNCE_DELAY_MS = 5;
};
