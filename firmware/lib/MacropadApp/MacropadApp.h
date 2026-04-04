#pragma once

#include <Arduino.h>
#include "Keymap.h"

class MacropadApp {
public:
    void begin();
    void run();

private:
    void capture_wake_key();
    void replay_wake_key_when_ready();
    bool find_pressed_key_position(uint8_t& out_col, uint8_t& out_row);
    void init_serial();
    void check_power_idle();
    void process_input_events();

    bool _ble_mode = false;
    bool _pending_wake_action_valid = false;
    KeyAction _pending_wake_action;
    unsigned long _last_power_check_ms = 0;
};
