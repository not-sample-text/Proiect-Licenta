/**
 * @file keymap.cpp
 * @brief Key mapping resolution implementation.
 */

#include "keymap.h"
#include "config.h"
#include "debug.h"
#include <cstring>

// ── Layer 0 Default Mapping (F13-F24) ──────────────────────────
// Hardcoded fallback for Layer 0 (3 cols × 4 rows = 12 keys)
static const uint8_t LAYER0_DEFAULT_KEYS[12] = {
    HID_KEY_F13, HID_KEY_F14, HID_KEY_F15,  // Row 0
    HID_KEY_F16, HID_KEY_F17, HID_KEY_F18,  // Row 1
    HID_KEY_F19, HID_KEY_F20, HID_KEY_F21,  // Row 2
    HID_KEY_F22, HID_KEY_F23, HID_KEY_F24   // Row 3
};

// ── Initialization ──────────────────────────────────────────────
void keymap_init() {
    DBG_INFO("Keymap system initialized");
}

// ── Key Mapping ─────────────────────────────────────────────────
bool keymap_get_action(Layer layer, uint8_t col, uint8_t row, KeyAction& action) {
    // Validate coordinates
    if (col >= 3 || row >= 4) {
        return false;
    }
    
    // Layer 0: Use hardcoded defaults
    if (layer == LAYER_0_FN_KEYS) {
        action.type = ACTION_HID_KEY;
        action.hid_key.keycode = LAYER0_DEFAULT_KEYS[row * 3 + col];
        action.hid_key.modifiers = 0;
        return true;
    }
    
    // Layers 2-3: Serial protocol (no config lookup needed)
    if (layer == LAYER_2_SERIAL || layer == LAYER_3_SERIAL) {
        action.type = ACTION_SERIAL_SEND;
        return true;
    }
    
    // Layer 1: Try to load from config
    const char* action_str = config_get_key_action(layer, col, row);
    if (action_str && keymap_parse_action(action_str, action)) {
        return true;
    }
    
    // Fallback for Layer 1: Use Layer 0 defaults
    DBG_VERBOSE("No config for Layer %d C%dR%d, using Layer 0 default", 
                layer, col, row);
    action.type = ACTION_HID_KEY;
    action.hid_key.keycode = LAYER0_DEFAULT_KEYS[row * 3 + col];
    action.hid_key.modifiers = 0;
    return true;
}

// ── Action String Parsing ───────────────────────────────────────
/**
 * Parse a simple keycode name (e.g., "F13", "A", "Enter")
 */
static bool parse_keycode(const char* name, uint8_t& keycode) {
    // Function keys F13-F24
    if (strncmp(name, "F", 1) == 0) {
        int fn = atoi(name + 1);
        if (fn >= 13 && fn <= 24) {
            keycode = HID_KEY_F13 + (fn - 13);
            return true;
        }
    }
    
    // Single letter A-Z (uppercase)
    if (strlen(name) == 1 && name[0] >= 'A' && name[0] <= 'Z') {
        keycode = HID_KEY_A + (name[0] - 'A');
        return true;
    }
    
    // Add more keycodes as needed (Enter, Tab, Space, etc.)
    
    return false;
}

/**
 * Parse a media key string (e.g., "Media:VolUp")
 */
static bool parse_media_key(const char* name, uint16_t& usage_code) {
    if (strcmp(name, "VolUp") == 0 || strcmp(name, "VolumeUp") == 0) {
        usage_code = HID_CONSUMER_VOLUME_UP;
        return true;
    }
    if (strcmp(name, "VolDown") == 0 || strcmp(name, "VolumeDown") == 0) {
        usage_code = HID_CONSUMER_VOLUME_DOWN;
        return true;
    }
    if (strcmp(name, "Mute") == 0) {
        usage_code = HID_CONSUMER_MUTE;
        return true;
    }
    if (strcmp(name, "Play") == 0 || strcmp(name, "PlayPause") == 0) {
        usage_code = HID_CONSUMER_PLAY_PAUSE;
        return true;
    }
    if (strcmp(name, "Next") == 0 || strcmp(name, "NextTrack") == 0) {
        usage_code = HID_CONSUMER_NEXT_TRACK;
        return true;
    }
    if (strcmp(name, "Prev") == 0 || strcmp(name, "PrevTrack") == 0) {
        usage_code = HID_CONSUMER_PREV_TRACK;
        return true;
    }
    
    return false;
}

bool keymap_parse_action(const char* action_str, KeyAction& action) {
    if (!action_str || strlen(action_str) == 0) {
        return false;
    }
    
    // Check for media key prefix "Media:"
    if (strncmp(action_str, "Media:", 6) == 0) {
        action.type = ACTION_HID_MEDIA;
        return parse_media_key(action_str + 6, action.hid_media.usage_code);
    }
    
    // Check for modifier combos (e.g., "Ctrl+Shift+C")
    char buffer[64];
    strncpy(buffer, action_str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    uint8_t modifiers = 0;
    uint8_t keycode = 0;
    
    char* token = strtok(buffer, "+");
    char* last_token = nullptr;
    
    while (token != nullptr) {
        last_token = token;
        
        // Check if it's a modifier
        if (strcmp(token, "Ctrl") == 0 || strcmp(token, "Control") == 0) {
            modifiers |= HID_MOD_CTRL;
        }
        else if (strcmp(token, "Shift") == 0) {
            modifiers |= HID_MOD_SHIFT;
        }
        else if (strcmp(token, "Alt") == 0) {
            modifiers |= HID_MOD_ALT;
        }
        else if (strcmp(token, "Gui") == 0 || strcmp(token, "Win") == 0 || strcmp(token, "Cmd") == 0) {
            modifiers |= HID_MOD_GUI;
        }
        
        token = strtok(nullptr, "+");
    }
    
    // Last token should be the main key
    if (last_token && parse_keycode(last_token, keycode)) {
        action.type = ACTION_HID_KEY;
        action.hid_key.keycode = keycode;
        action.hid_key.modifiers = modifiers;
        return true;
    }
    
    // If no modifiers, try parsing the whole string as a keycode
    if (modifiers == 0 && parse_keycode(action_str, keycode)) {
        action.type = ACTION_HID_KEY;
        action.hid_key.keycode = keycode;
        action.hid_key.modifiers = 0;
        return true;
    }
    
    return false;
}
