#pragma once

#include <Arduino.h>

/**
 * @brief Simple hardcoded keymap for the 3x4 matrix.
 */
class Keymap {
public:
    static const char* getLabel(uint8_t layer, uint8_t row, uint8_t col);
    static uint8_t getKeyCode(uint8_t layer, uint8_t row, uint8_t col);
};
