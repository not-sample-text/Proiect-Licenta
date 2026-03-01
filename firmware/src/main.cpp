/**
 * @file main.cpp
 * @brief Entry point for the PROS3 Macropad firmware.
 *
 * This is the Phase-1 skeleton. It initialises every GPIO defined
 * in pins.h and blinks the status LED to confirm the board is alive.
 * Subsystems will be wired in incrementally from here.
 */

#include <Arduino.h>
#include "pins.h"
#include "debug.h"
#include "protocol.h"
#include "power.h"
#include "input_events.h"
#include "matrix.h"
#include "encoder.h"
#include "config.h"
#include "layers.h"
#include "keymap.h"
#include "usb_hid.h"
#include "ble_hid.h"
#include "oled.h"
#include "rgb.h"

// ── Global State ────────────────────────────────────────────────
bool g_debug_enabled = true;  // Runtime debug toggle (default: ON)
bool g_ble_mode = false;       // BLE mode active (vs USB mode)

// ── Forward declarations for subsystem init stubs ───────────────
static void init_pins();
static void init_serial();
static void process_serial_commands();
static void process_input_events();
static void check_power_idle();

// ════════════════════════════════════════════════════════════════
void setup() {
    init_serial();

    DBG_INFO("MAIN", "┌──────────────────────────────┐");
    DBG_INFO("MAIN", "│   PROS3 Macropad — Booting   │");
    DBG_INFO("MAIN", "└──────────────────────────────┘");

    init_pins();
    power_init();
    
    // Initialize configuration and layer system
    config_init();      // Load config.json from LittleFS
    layers_init();      // Initialize layer state machine
    keymap_init();      // Initialize keymap resolver
    
    // Initialize input subsystems
    matrix_init();
    encoder_init();
    
    // Initialize display and lighting
    oled_init();
    rgb_init();
    
    // Initialize BLE if mode switch is set
    g_ble_mode = ble_is_enabled();
    if (g_ble_mode) {
        DBG_INFO("MAIN", "BLE mode selected");
        ble_hid_init();
        oled_set_connection_mode("BLE");
    } else {
        DBG_INFO("MAIN", "USB mode selected");
        oled_set_connection_mode("USB");
    }
    
    // Set initial encoder mode on OLED
    oled_set_encoder_mode("VOLUME");  // Default encoder mode
    oled_set_layer(layers_get_name(layers_get_current()));  // Set initial layer

    // Quick blink to confirm life
    for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_LED_STATUS, HIGH);
        delay(100);
        digitalWrite(PIN_LED_STATUS, LOW);
        delay(100);
    }

    setup_usb_hid();

    DBG_INFO("MAIN", "Boot complete — entering main loop");
}

// ════════════════════════════════════════════════════════════════
void loop() {
    // Scan input devices
    matrix_scan();
    encoder_process();
    
    // Process any queued input events
    process_input_events();
    
    // Process incoming serial commands (debug toggle, config upload, etc.)
    process_serial_commands();
    
    // Check power management
    check_power_idle();
    
    // Update display and lighting
    oled_update();
    rgb_update();
    
    // Handle HID task based on mode
    if (g_ble_mode) {
        ble_hid_task();
    } else {
        usb_hid_task();
    }

    delay(1);   // yield to RTOS; will be replaced by proper scheduling
}

// ── Pin initialisation ──────────────────────────────────────────
static void init_pins() {
    DBG_INFO("MAIN", "Initializing GPIOs...");

    // Status LED
    pinMode(PIN_LED_STATUS, OUTPUT);
    digitalWrite(PIN_LED_STATUS, LOW);

    // Key matrix — columns as OUTPUT (active-low scan), rows as INPUT (read)
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        pinMode(COL_PINS[c], OUTPUT);
        digitalWrite(COL_PINS[c], HIGH);  // idle high
    }
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        pinMode(ROW_PINS[r], INPUT);      // external pull-ups on PCB
    }
    DBG_VERBOSE("MAIN", "  Key matrix: %d cols × %d rows", MATRIX_COLS, MATRIX_ROWS);

    // Rotary encoder
    pinMode(PIN_ENC_CLK, INPUT);          // external pull-up
    pinMode(PIN_ENC_DT,  INPUT);          // external pull-up
    pinMode(PIN_ENC_SW,  INPUT);          // external pull-up
    DBG_VERBOSE("MAIN", "  Encoder: CLK=%d  DT=%d  SW=%d", PIN_ENC_CLK, PIN_ENC_DT, PIN_ENC_SW);

    // OLED — I²C pins are configured by Wire.begin(), listed here for clarity
    DBG_VERBOSE("MAIN", "  OLED I2C: SDA=%d  SCL=%d", PIN_OLED_SDA, PIN_OLED_SCL);

    // RGB LED data pin
    pinMode(PIN_RGB_LEDS, OUTPUT);
    digitalWrite(PIN_RGB_LEDS, LOW);
    DBG_VERBOSE("MAIN", "  RGB data pin: %d  (%d LEDs)", PIN_RGB_LEDS, RGB_LED_COUNT);

    // BT / USB mode switch
    pinMode(PIN_BT_SELECT, INPUT);        // external pull-up
    DBG_VERBOSE("MAIN", "  BT select: %d", PIN_BT_SELECT);

    // VBUS sensing
    pinMode(PIN_VBUS_SENSE, INPUT);
    DBG_VERBOSE("MAIN", "  VBUS sense: %d", PIN_VBUS_SENSE);

    DBG_INFO("MAIN", "GPIOs initialised OK");
}

// ── Serial / USB CDC ────────────────────────────────────────────
static void init_serial() {
    Serial.begin(115200);

    // Give USB CDC a moment to enumerate on the host
    uint32_t t0 = millis();
    while (!Serial && (millis() - t0 < 2000)) {
        delay(10);
    }
}

// ── Serial Command Processing ───────────────────────────────────
/**
 * Process incoming serial commands.
 * Currently supports:
 *   - DBG:ON\n  — enable debug output
 *   - DBG:OFF\n — disable debug output
 *   - CFG:START:<size>\n — begin config upload
 *   - CFG:DATA:<hex_data>\n — receive config data chunk
 *   - CFG:END\n — finalize config upload
 * 
 * Commands are prefixed with their category (DBG:, CFG:, etc.)
 * to avoid collision with protocol data.
 */

// Config upload state
enum ConfigState {
    CFG_IDLE,
    CFG_RECEIVING,
};
static ConfigState g_cfg_state = CFG_IDLE;
static String g_cfg_buffer;
static size_t g_cfg_expected_size = 0;

static void process_serial_commands() {
    static String cmd_buffer;
    
    while (Serial.available()) {
        char c = Serial.read();
        
        if (c == '\n') {
            // Process complete command
            cmd_buffer.trim();
            
            if (cmd_buffer == "DBG:ON") {
                g_debug_enabled = true;
                Serial.println("#Debug output: ENABLED");
            }
            else if (cmd_buffer == "DBG:OFF") {
                Serial.println("#Debug output: DISABLED");
                g_debug_enabled = false;
            }
            else if (cmd_buffer.startsWith("DBG:")) {
                Serial.println("#Unknown debug command");
            }
            // Config upload commands
            else if (cmd_buffer.startsWith("CFG:START:")) {
                // Extract size
                String size_str = cmd_buffer.substring(10);
                g_cfg_expected_size = size_str.toInt();
                
                if (g_cfg_expected_size > 0 && g_cfg_expected_size <= CONFIG_MAX_SIZE) {
                    g_cfg_state = CFG_RECEIVING;
                    g_cfg_buffer = "";
                    g_cfg_buffer.reserve(g_cfg_expected_size);
                    Serial.printf("#CFG:READY:%u\n", g_cfg_expected_size);
                    DBG_INFO("MAIN", "Config upload started: %u bytes expected", g_cfg_expected_size);
                } else {
                    Serial.println("#CFG:ERROR:Invalid size");
                }
            }
            else if (cmd_buffer.startsWith("CFG:DATA:") && g_cfg_state == CFG_RECEIVING) {
                // Append data to buffer
                String data = cmd_buffer.substring(9);
                g_cfg_buffer += data;
                Serial.printf("#CFG:ACK:%u/%u\n", g_cfg_buffer.length(), g_cfg_expected_size);
            }
            else if (cmd_buffer == "CFG:END" && g_cfg_state == CFG_RECEIVING) {
                // Finalize upload
                if (g_cfg_buffer.length() == g_cfg_expected_size) {
                    if (config_save(g_cfg_buffer.c_str(), g_cfg_buffer.length())) {
                        Serial.println("#CFG:SUCCESS");
                        DBG_INFO("MAIN", "Config saved and reloaded");
                        
                        // Reload RGB settings
                        const RGBConfig& cfg = config_get_rgb();
                        rgb_set_mode((RGBMode)cfg.mode);
                        rgb_set_brightness(cfg.brightness);
                        rgb_set_speed(cfg.speed);
                        rgb_set_color((cfg.color >> 16) & 0xFF, 
                                     (cfg.color >> 8) & 0xFF, 
                                     cfg.color & 0xFF);
                        
                        oled_show_status("Config Updated", 2000);
                    } else {
                        Serial.println("#CFG:ERROR:Save failed");
                        DBG_ERROR("MAIN", "Config save failed");
                    }
                } else {
                    Serial.printf("#CFG:ERROR:Size mismatch %u/%u\n", 
                                 g_cfg_buffer.length(), g_cfg_expected_size);
                }
                
                // Reset state
                g_cfg_state = CFG_IDLE;
                g_cfg_buffer = "";
                g_cfg_expected_size = 0;
            }
            else if (cmd_buffer == "CFG:ABORT") {
                g_cfg_state = CFG_IDLE;
                g_cfg_buffer = "";
                g_cfg_expected_size = 0;
                Serial.println("#CFG:ABORTED");
            }
            
            cmd_buffer = "";
        }
        else if (c >= 32 && c < 127) {  // printable ASCII
            cmd_buffer += c;
            
            // Prevent buffer overflow
            if (cmd_buffer.length() > 256) {
                cmd_buffer = "";
                Serial.println("#Error: Command too long");
            }
        }
    }
}

// ── Power Management Idle Check ─────────────────────────────────
static void check_power_idle() {
    static uint32_t last_check_ms = 0;
    
    // Check every 1 second (no need to check more frequently)
    if (millis() - last_check_ms < 1000) {
        return;
    }
    last_check_ms = millis();
    
    // Check idle timeout and transition power modes as needed
    power_check_idle();
}

// ── Input Event Processing ──────────────────────────────────────
/**
 * Process all queued input events.
 * 
 * Behavior per layer:
 * - Encoder button: Cycles layers (all layers)
 * - Layer 0 (FN Keys): Send HID keyboard reports (F13-F24)
 * - Layer 1 (User): Send HID keyboard/media reports (from config)
 * - Layers 2-3 (Serial): Send protocol bytes to host listener
 */
static void process_input_events() {
    InputEvent event;
    
    while (g_input_queue.dequeue(event)) {
        // Handle encoder button — toggles encoder mode
        if (event.type == EVENT_ENCODER_BUTTON_PRESS) {
            EncoderMode new_mode = encoder_toggle_mode();
            const char* mode_str = (new_mode == ENCODER_MODE_VOLUME) ? "VOLUME" : "LAYER";
            oled_set_encoder_mode(mode_str);
            DBG_INFO("MAIN", "Encoder mode: %s", mode_str);
            continue;
        }
        
        // Handle encoder rotation
        if (event.type == EVENT_ENCODER_CW || event.type == EVENT_ENCODER_CCW) {
            EncoderMode current_mode = encoder_get_mode();
            bool is_cw = (event.type == EVENT_ENCODER_CW);
            
            if (current_mode == ENCODER_MODE_VOLUME) {
                // Volume control mode
                DBG_VERBOSE("MAIN", "Encoder %s (volume control)", is_cw ? "CW" : "CCW");
                if (g_ble_mode) {
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
                // Layer cycling mode
                DBG_VERBOSE("MAIN", "Encoder %s (layer cycling)", is_cw ? "CW" : "CCW");
                Layer new_layer = is_cw ? layers_cycle_next() : layers_cycle_prev();
                oled_set_layer(layers_get_name(new_layer));
            }
            continue;
        }
        
        // Handle key press/release
        if (event.type == EVENT_KEY_PRESS || event.type == EVENT_KEY_RELEASE) {
            Layer current_layer = layers_get_current();
            bool is_press = (event.type == EVENT_KEY_PRESS);
            
            // Update OLED with key label on press
            if (is_press) {
                const char* label = config_get_key_label(current_layer, event.col, event.row);
                if (label != nullptr) {
                    oled_set_last_key(label);
                } else if (current_layer == LAYER_0_FN_KEYS) {
                    // Fallback for FN keys layer: F13-F24
                    char fallback_label[8];
                    uint8_t fn_num = 13 + event.row * 3 + event.col;
                    snprintf(fallback_label, sizeof(fallback_label), "F%d", fn_num);
                    oled_set_last_key(fallback_label);
                }
            }
            
            // Resolve action for this key
            KeyAction action;
            if (!keymap_get_action(current_layer, event.col, event.row, action)) {
                DBG_WARN("MAIN", "No action for Layer %s C%dR%d", 
                        layers_get_name(current_layer), event.col, event.row);
                continue;
            }
            
            // Dispatch based on action type
            // Move variable declaration outside the switch
            EventType proto_evt;
                switch (action.type) {
                case ACTION_HID_KEY:
                    DBG_INFO("MAIN", "%s: HID Key 0x%02X (mods: 0x%02X) — C%dR%d [%s]",
                        is_press ? "PRESS" : "RELEASE",
                        action.hid_key.keycode,
                        action.hid_key.modifiers,
                        event.col, event.row,
                        layers_get_name(current_layer));
                    if (g_ble_mode) {
                        ble_send_key(action.hid_key.keycode, action.hid_key.modifiers, is_press);
                    } else {
                        hid_send_key(action.hid_key.keycode, action.hid_key.modifiers, is_press);
                    }
                    break;
                    
                case ACTION_HID_MEDIA:
                    if (is_press) {  // Media keys only on press
                        DBG_INFO("MAIN", "PRESS: HID Media 0x%04X — C%dR%d [%s]",
                                action.hid_media.usage_code,
                                event.col, event.row,
                                layers_get_name(current_layer));
                        if (g_ble_mode) {
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
                    // Send protocol byte
                    protocol_send(Serial, current_layer, event.col, event.row, proto_evt);
                    break;
                    
                case ACTION_NONE:
                    DBG_INFO("MAIN", "No action for this event");
                    break;
            }
        }
    }
}
