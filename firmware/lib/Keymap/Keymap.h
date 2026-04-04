/**
 * @file keymap.h
 * @brief Key mapping resolution for the PROS3 Macropad.
 *
 * Resolves what action should be performed when a key is pressed,
 * based on the current layer and the key coordinates.
 *
 * Action types:
 * - HID Keycode: Send USB/BLE keyboard report (layers 0-1)
 * - Media Key: Send consumer control report (layers 0-1)
 * - Serial Protocol: Send encoded byte to host (layers 2-3)
 *
 * Responsibilities:
 * - Resolve (layer, col, row) → KeyAction
 * - Handle Layer 0 defaults (F13-F24 hardcoded)
 * - Parse action strings from config
 * - Provide fallback behavior when config is missing
 */

#pragma once

#include <Arduino.h>
#include "Layers.h"

// ── Action Types ────────────────────────────────────────────────
enum ActionType : uint8_t {
    ACTION_NONE = 0,        // No action defined
    
    // HID actions (layers 0-1)
    ACTION_HID_KEY,         // Single key or modifier combo (e.g., "F13", "Ctrl+C")
    ACTION_HID_MEDIA,       // Media/consumer key (e.g., "Media:VolUp")
    
    // Serial protocol actions (layers 2-3)
    ACTION_SERIAL_SEND,     // Send protocol byte to host listener
};

// ── Key Action Structure ────────────────────────────────────────
struct KeyAction {
    ActionType type;
    
    union {
        // For ACTION_HID_KEY
        struct {
            uint8_t keycode;       // Primary keycode
            uint8_t modifiers;     // Modifier mask (Ctrl=0x01, Shift=0x02, Alt=0x04, GUI=0x08)
        } hid_key;
        
        // For ACTION_HID_MEDIA
        struct {
            uint16_t usage_code;   // HID Consumer Control usage code
        } hid_media;
        
        // For ACTION_SERIAL_SEND
        struct {
            // No additional data needed — protocol byte is generated
            // from (layer, col, row) at send time
        } serial;
    };
    
    KeyAction()
        : type(ACTION_NONE)
    {
        hid_key.keycode = 0;
        hid_key.modifiers = 0;
    }
};

// ── HID Key Codes (subset) ──────────────────────────────────────
// Full list: https://www.usb.org/sites/default/files/hut1_2.pdf
#define HID_KEY_A           0x04
#define HID_KEY_B           0x05
#define HID_KEY_C           0x06
// ... (add more as needed)
#define HID_KEY_F13         0x68
#define HID_KEY_F14         0x69
#define HID_KEY_F15         0x6A
#define HID_KEY_F16         0x6B
#define HID_KEY_F17         0x6C
#define HID_KEY_F18         0x6D
#define HID_KEY_F19         0x6E
#define HID_KEY_F20         0x6F
#define HID_KEY_F21         0x70
#define HID_KEY_F22         0x71
#define HID_KEY_F23         0x72
#define HID_KEY_F24         0x73

// Modifier masks
#define HID_MOD_CTRL        0x01
#define HID_MOD_SHIFT       0x02
#define HID_MOD_ALT         0x04
#define HID_MOD_GUI         0x08  // Windows/Command key

// ── HID Consumer Control Codes ──────────────────────────────────
#define HID_CONSUMER_VOLUME_UP      0xE9
#define HID_CONSUMER_VOLUME_DOWN    0xEA
#define HID_CONSUMER_MUTE           0xE2
#define HID_CONSUMER_PLAY_PAUSE     0xCD
#define HID_CONSUMER_NEXT_TRACK     0xB5
#define HID_CONSUMER_PREV_TRACK     0xB6

// ── Key Mapping ─────────────────────────────────────────────────
/**
 * Initialize the keymap system.
 * Must be called after config_init().
 */
void keymap_init();

/**
 * Resolve the action for a specific key on the current layer.
 * 
 * @param layer Current layer
 * @param col Key column (0-2)
 * @param row Key row (0-3)
 * @param[out] action The resolved action
 * @return true if an action was resolved, false if no mapping exists
 */
bool keymap_get_action(Layer layer, uint8_t col, uint8_t row, KeyAction& action);

// ── Action String Parsing ───────────────────────────────────────
/**
 * Parse an action string from config into a KeyAction.
 * 
 * Examples:
 *   "F13"              → ACTION_HID_KEY, F13
 *   "Ctrl+Shift+C"     → ACTION_HID_KEY, C with modifiers
 *   "Media:VolUp"      → ACTION_HID_MEDIA, volume up
 *   "A"                → ACTION_HID_KEY, A
 * 
 * @param action_str The config string
 * @param[out] action The parsed action
 * @return true if parsed successfully
 */
bool keymap_parse_action(const char* action_str, KeyAction& action);
