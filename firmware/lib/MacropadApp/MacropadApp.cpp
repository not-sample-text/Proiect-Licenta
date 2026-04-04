#include "MacropadApp.h"

#include <Arduino.h>
#include <esp_sleep.h>
#include "debug.h"
#include "protocol.h"
#include "pins.h"
#include "Power.h"
#include "InputEvents.h"
#include "Matrix.h"
#include "Encoder.h"
#include "Config.h"
#include "Layers.h"
#include "Keymap.h"
#include "UsbHid.h"
#include "BleHid.h"
#include "Oled.h"
#include "Rgb.h"
#include "BoardSupport.h"
#include "SerialControl.h"

bool g_debug_enabled = true;

namespace {
BoardSupport g_board_support;
SerialControl g_serial_control;
}

void MacropadApp::begin() {
    init_serial();

    DBG_INFO("MAIN", "┌──────────────────────────────┐");
    DBG_INFO("MAIN", "│   PROS3 Macropad — Booting   │");
    DBG_INFO("MAIN", "└──────────────────────────────┘");

    g_board_support.begin();
    power_init();

    config_init();
    layers_init();
    keymap_init();

    matrix_init();
    encoder_init();

    oled_init();
    rgb_init();

    _ble_mode = ble_is_enabled();
    if (_ble_mode) {
        DBG_INFO("MAIN", "BLE mode selected");
        ble_hid_init();
        oled_set_connection_mode("BLE");
    } else {
        DBG_INFO("MAIN", "USB mode selected");
        oled_set_connection_mode("USB");
    }

    oled_set_encoder_mode("VOLUME");
    oled_set_layer(layers_get_name(layers_get_current()));

    g_board_support.boot_blink();

    setup_usb_hid();
    g_serial_control.begin();

    capture_wake_key();

    DBG_INFO("MAIN", "Boot complete — entering main loop");
}

void MacropadApp::run() {
    matrix_scan();
    encoder_process();

    process_input_events();
    g_serial_control.run();

    if (power_consume_shutdown_request()) {
        DBG_INFO("MAIN", "User requested shutdown mode");
        power_enter_shutdown_sleep();
        return;
    }

    check_power_idle();

    oled_update();
    rgb_update();

    if (_ble_mode) {
        ble_hid_task();
    } else {
        usb_hid_task();
    }

    replay_wake_key_when_ready();

    delay(1);
}

bool MacropadApp::find_pressed_key_position(uint8_t& out_col, uint8_t& out_row) {
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        digitalWrite(COL_PINS[c], HIGH);
    }

    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        digitalWrite(COL_PINS[c], LOW);
        delayMicroseconds(5);

        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            if (digitalRead(ROW_PINS[r]) == LOW) {
                out_col = c;
                out_row = r;

                for (uint8_t restore_col = 0; restore_col < MATRIX_COLS; restore_col++) {
                    digitalWrite(COL_PINS[restore_col], HIGH);
                }
                return true;
            }
        }

        digitalWrite(COL_PINS[c], HIGH);
    }

    return false;
}

void MacropadApp::capture_wake_key() {
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_EXT1) {
        return;
    }

    uint8_t wake_col = 0;
    uint8_t wake_row = 0;
    if (!find_pressed_key_position(wake_col, wake_row)) {
        return;
    }

    Layer current_layer = layers_get_current();
    KeyAction action;
    if (!keymap_get_action(current_layer, wake_col, wake_row, action)) {
        return;
    }

    _pending_wake_action = action;
    _pending_wake_action_valid = true;
    DBG_INFO("MAIN", "Wake key captured at C%dR%d", wake_col, wake_row);
}

void MacropadApp::replay_wake_key_when_ready() {
    if (!_pending_wake_action_valid) {
        return;
    }

    const bool transport_ready = _ble_mode ? ble_is_ready() : usb_hid_is_ready();
    if (!transport_ready) {
        return;
    }

    switch (_pending_wake_action.type) {
        case ACTION_HID_KEY:
            if (_ble_mode) {
                ble_send_key(_pending_wake_action.hid_key.keycode, _pending_wake_action.hid_key.modifiers, true);
                ble_send_key(_pending_wake_action.hid_key.keycode, _pending_wake_action.hid_key.modifiers, false);
            } else {
                hid_send_key(_pending_wake_action.hid_key.keycode, _pending_wake_action.hid_key.modifiers, true);
                hid_send_key(_pending_wake_action.hid_key.keycode, _pending_wake_action.hid_key.modifiers, false);
            }
            break;

        case ACTION_HID_MEDIA:
            if (_ble_mode) {
                ble_send_consumer(_pending_wake_action.hid_media.usage_code);
            } else {
                send_consumer_report(_pending_wake_action.hid_media.usage_code);
                send_consumer_report(0);
            }
            break;

        case ACTION_SERIAL_SEND:
        case ACTION_NONE:
        default:
            break;
    }

    DBG_INFO("MAIN", "Replayed captured wake key");
    _pending_wake_action_valid = false;
}

void MacropadApp::init_serial() {
    Serial.begin(115200);

    const unsigned long start_ms = millis();
    while (!Serial && (millis() - start_ms < 2000)) {
        delay(10);
    }
}

void MacropadApp::check_power_idle() {
    if (millis() - _last_power_check_ms < 1000) {
        return;
    }

    _last_power_check_ms = millis();
    power_check_idle();
}

void MacropadApp::process_input_events() {
    InputEvent event;

    while (g_input_queue.dequeue(event)) {
        if (event.type == EVENT_ENCODER_BUTTON_PRESS) {
            EncoderMode new_mode = encoder_toggle_mode();
            const char* mode_str = (new_mode == ENCODER_MODE_VOLUME) ? "VOLUME" : "LAYER";
            oled_set_encoder_mode(mode_str);
            DBG_INFO("MAIN", "Encoder mode: %s", mode_str);
            continue;
        }

        if (event.type == EVENT_ENCODER_CW || event.type == EVENT_ENCODER_CCW) {
            EncoderMode current_mode = encoder_get_mode();
            bool is_cw = (event.type == EVENT_ENCODER_CW);

            if (current_mode == ENCODER_MODE_VOLUME) {
                DBG_VERBOSE("MAIN", "Encoder %s (volume control)", is_cw ? "CW" : "CCW");
                if (_ble_mode) {
                    if (is_cw) {
                        ble_volume_up();
                    } else {
                        ble_volume_down();
                    }
                } else {
                    if (is_cw) {
                        hid_volume_up();
                    } else {
                        hid_volume_down();
                    }
                }
            } else {
                DBG_VERBOSE("MAIN", "Encoder %s (layer cycling)", is_cw ? "CW" : "CCW");
                Layer new_layer = is_cw ? layers_cycle_next() : layers_cycle_prev();
                oled_set_layer(layers_get_name(new_layer));
            }
            continue;
        }

        if (event.type == EVENT_KEY_PRESS || event.type == EVENT_KEY_RELEASE) {
            Layer current_layer = layers_get_current();
            bool is_press = (event.type == EVENT_KEY_PRESS);

            if (is_press) {
                const char* label = config_get_key_label(current_layer, event.col, event.row);
                if (label != nullptr) {
                    oled_set_last_key(label);
                } else if (current_layer == LAYER_0_FN_KEYS) {
                    char fallback_label[8];
                    uint8_t fn_num = 13 + event.row * 3 + event.col;
                    snprintf(fallback_label, sizeof(fallback_label), "F%d", fn_num);
                    oled_set_last_key(fallback_label);
                }
            }

            KeyAction action;
            if (!keymap_get_action(current_layer, event.col, event.row, action)) {
                DBG_WARN("MAIN", "No action for Layer %s C%dR%d",
                         layers_get_name(current_layer), event.col, event.row);
                continue;
            }

            EventType proto_evt;
            switch (action.type) {
                case ACTION_HID_KEY:
                    DBG_INFO("MAIN", "%s: HID Key 0x%02X (mods: 0x%02X) — C%dR%d [%s]",
                             is_press ? "PRESS" : "RELEASE",
                             action.hid_key.keycode,
                             action.hid_key.modifiers,
                             event.col, event.row,
                             layers_get_name(current_layer));
                    if (_ble_mode) {
                        ble_send_key(action.hid_key.keycode, action.hid_key.modifiers, is_press);
                    } else {
                        hid_send_key(action.hid_key.keycode, action.hid_key.modifiers, is_press);
                    }
                    break;

                case ACTION_HID_MEDIA:
                    if (is_press) {
                        DBG_INFO("MAIN", "PRESS: HID Media 0x%04X — C%dR%d [%s]",
                                 action.hid_media.usage_code,
                                 event.col, event.row,
                                 layers_get_name(current_layer));
                        if (_ble_mode) {
                            ble_send_consumer(action.hid_media.usage_code);
                        } else {
                            send_consumer_report(action.hid_media.usage_code);
                        }
                    }
                    break;

                case ACTION_SERIAL_SEND:
                    DBG_INFO("MAIN", "%s: Serial protocol — C%dR%d [%s]",
                             is_press ? "PRESS" : "RELEASE",
                             event.col, event.row,
                             layers_get_name(current_layer));
                    proto_evt = is_press ? EVT_KEY_PRESS : EVT_KEY_RELEASE;
                    protocol_send(Serial, current_layer, event.col, event.row, proto_evt);
                    break;

                case ACTION_NONE:
                    DBG_INFO("MAIN", "No action for this event");
                    break;
            }
        }
    }
}
