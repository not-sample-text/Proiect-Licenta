#pragma once

#include <Arduino.h>

/**
 * @brief Handles SK6812 RGB LEDs using FastLED.
 */
class RgbHandler {
public:
    static void begin();
    static void run();

private:
    static uint32_t lastUpdate;
    static uint8_t hue;
};
