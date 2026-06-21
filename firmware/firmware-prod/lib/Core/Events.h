#pragma once

#include <Arduino.h>

/**
 * @brief Input event types for the macropad.
 */
enum class EventType : uint8_t {
    KeyPress,
    KeyRelease,
    EncoderCW,
    EncoderCCW,
    EncoderButton
};

/**
 * @brief Represents a single input event.
 */
struct InputEvent {
    EventType type;
    uint8_t   row;       // Matrix row (ignored for encoder events)
    uint8_t   col;       // Matrix column (ignored for encoder events)
    uint32_t  timestamp; // millis() when event occurred
};
