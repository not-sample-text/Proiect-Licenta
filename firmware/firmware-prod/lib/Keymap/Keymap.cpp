#include "Keymap.h"

// Example hardcoded labels and keycodes
const char* Keymap::getLabel(uint8_t layer, uint8_t row, uint8_t col) {
    static const char* labels[3][4] = {
        {"F13", "F14", "F15"},
        {"F16", "F17", "F18"},
        {"F19", "F20", "F21"},
        {"F22", "F23", "F24"}
    };
    if (row < 4 && col < 3) return labels[row][col];
    return "";
}

uint8_t Keymap::getKeyCode(uint8_t layer, uint8_t row, uint8_t col) {
    // Return F13-F24 keycodes (starting from 0x68 for F13)
    if (row < 4 && col < 3) return 0x68 + (row * 3) + col;
    return 0;
}
