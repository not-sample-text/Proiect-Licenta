#pragma once
#include <Arduino.h>

enum class EventType : uint8_t {
    KeyPress,
    KeyRelease,
    EncoderCW,
    EncoderCCW,
    EncoderButton
};

struct InputEvent {
    EventType type;
    uint8_t   row;
    static_cast<uint8_t> col;
    char      rawChar;
    uint32_t  timestamp;
};
