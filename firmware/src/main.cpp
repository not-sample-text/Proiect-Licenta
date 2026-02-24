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

// ── Global State ────────────────────────────────────────────────
bool g_debug_enabled = true;  // Runtime debug toggle (default: ON)

// ── Forward declarations for subsystem init stubs ───────────────
static void init_pins();
static void init_serial();
static void process_serial_commands();
static void process_input_events();
static void check_power_idle();

// ════════════════════════════════════════════════════════════════
void setup() {
    init_serial();

    DBG_INFO("┌──────────────────────────────┐");
    DBG_INFO("│   PROS3 Macropad — Booting   │");
    DBG_INFO("└──────────────────────────────┘");

    init_pins();
    power_init();
    
    // Initialize configuration and layer system
    config_init();      // Load config.json from LittleFS
    layers_init();      // Initialize layer state machine
    keymap_init();      // Initialize keymap resolver
    
    // Initialize input subsystems
    matrix_init();
    encoder_init();

    // Quick blink to confirm life
    for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_LED_STATUS, HIGH);
        delay(100);
        digitalWrite(PIN_LED_STATUS, LOW);
        delay(100);
    }

    setup_usb_hid();

    DBG_INFO("Boot complete — entering main loop");
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
    
    // Check for power idle timeout and sleep if needed
    check_power_idle();
    
    // TODO: poll / event-driven subsystems will go here
    //   - OLED refresh
    //   - Lighting update

    delay(1);   // yield to RTOS; will be replaced by proper scheduling
    usb_hid_task();
}

// ── Pin initialisation ──────────────────────────────────────────
static void init_pins() {
    DBG_INFO("Initializing GPIOs...");

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
    DBG_VERBOSE("  Key matrix: %d cols × %d rows", MATRIX_COLS, MATRIX_ROWS);

    // Rotary encoder
    pinMode(PIN_ENC_CLK, INPUT);          // external pull-up
    pinMode(PIN_ENC_DT,  INPUT);          // external pull-up
    pinMode(PIN_ENC_SW,  INPUT);          // external pull-up
    DBG_VERBOSE("  Encoder: CLK=%d  DT=%d  SW=%d", PIN_ENC_CLK, PIN_ENC_DT, PIN_ENC_SW);

    // OLED — I²C pins are configured by Wire.begin(), listed here for clarity
    DBG_VERBOSE("  OLED I2C: SDA=%d  SCL=%d", PIN_OLED_SDA, PIN_OLED_SCL);

    // RGB LED data pin
    pinMode(PIN_RGB_LEDS, OUTPUT);
    digitalWrite(PIN_RGB_LEDS, LOW);
    DBG_VERBOSE("  RGB data pin: %d  (%d LEDs)", PIN_RGB_LEDS, RGB_LED_COUNT);

    // BT / USB mode switch
    pinMode(PIN_BT_SELECT, INPUT);        // external pull-up
    DBG_VERBOSE("  BT select: %d", PIN_BT_SELECT);

    // VBUS sensing
    pinMode(PIN_VBUS_SENSE, INPUT);
    DBG_VERBOSE("  VBUS sense: %d", PIN_VBUS_SENSE);

    DBG_INFO("GPIOs initialised OK");
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
 * 
 * Commands are prefixed with their category (DBG:, CFG:, etc.)
 * to avoid collision with protocol data.
 */
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
            // Future: CFG:... for config upload, etc.
            
            cmd_buffer = "";
        }
        else if (c >= 32 && c < 127) {  // printable ASCII
            cmd_buffer += c;
            
            // Prevent buffer overflow
            if (cmd_buffer.length() > 128) {
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
    
    // Check if we should enter sleep mode
    if (power_should_sleep()) {
        power_enter_sleep();
        // Execution resumes here after wake
    }
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
        // Handle encoder button — cycles layers
        if (event.type == EVENT_ENCODER_BUTTON_PRESS) {
            Layer new_layer = layers_cycle_next();
            DBG_INFO("Layer switched: %s", layers_get_name(new_layer));
            // OLED will be updated via layer change callback (future)
            continue;
        }
        
        // Handle encoder rotation
        if (event.type == EVENT_ENCODER_CW || event.type == EVENT_ENCODER_CCW) {
            // On layers 0-1: Volume control (media keys)
            // On layers 2-3: Send protocol byte
            Layer current_layer = layers_get_current();
            
            if (current_layer == LAYER_0_FN_KEYS || current_layer == LAYER_1_USER) {
                DBG_VERBOSE("Encoder %s (volume control)", 
                            event.type == EVENT_ENCODER_CW ? "CW" : "CCW");
                // TODO: Send HID consumer control report
                //   hid_send_media(event.type == EVENT_ENCODER_CW 
                //                  ? HID_CONSUMER_VOLUME_UP 
                //                  : HID_CONSUMER_VOLUME_DOWN);
            } else {
                // Layers 2-3: Send encoder event via protocol
                DBG_VERBOSE("Encoder %s (serial protocol)", 
                            event.type == EVENT_ENCODER_CW ? "CW" : "CCW");
                // TODO: protocol_send(Serial, current_layer, 0, 0, 
                //                     event.type == EVENT_ENCODER_CW 
                //                     ? EVT_ENC_CW : EVT_ENC_CCW);
            }
            continue;
        }
        
        // Handle key press/release
        if (event.type == EVENT_KEY_PRESS || event.type == EVENT_KEY_RELEASE) {
            Layer current_layer = layers_get_current();
            
            // Resolve action for this key
            KeyAction action;
            if (!keymap_get_action(current_layer, event.col, event.row, action)) {
                DBG_WARN("No action for Layer %s C%dR%d", 
                        layers_get_name(current_layer), event.col, event.row);
                continue;
            }
            
            // Dispatch based on action type
            bool is_press = (event.type == EVENT_KEY_PRESS);
            
            // Move variable declaration outside the switch
            EventType proto_evt;
            switch (action.type) {
                case ACTION_HID_KEY:
                    DBG_INFO("%s: HID Key 0x%02X (mods: 0x%02X) — C%dR%d [%s]",
                            is_press ? "PRESS" : "RELEASE",
                            action.hid_key.keycode,
                            action.hid_key.modifiers,
                            event.col, event.row,
                            layers_get_name(current_layer));
                    // TODO: hid_send_key(action.hid_key.keycode, 
                    //                    action.hid_key.modifiers, is_press);
                    break;
                    
                case ACTION_HID_MEDIA:
                    if (is_press) {  // Media keys only on press
                        DBG_INFO("PRESS: HID Media 0x%04X — C%dR%d [%s]",
                                action.hid_media.usage_code,
                                event.col, event.row,
                                layers_get_name(current_layer));
                        // TODO: hid_send_media(action.hid_media.usage_code);
                    }
                    break;
                    
                case ACTION_SERIAL_SEND:
                    DBG_INFO("%s: Serial protocol — C%dR%d [%s]",
                            is_press ? "PRESS" : "RELEASE",
                            event.col, event.row,
                            layers_get_name(current_layer));
                    proto_evt = is_press ? EVT_KEY_PRESS : EVT_KEY_RELEASE;
                    // Send protocol byte
                    protocol_send(Serial, current_layer, event.col, event.row, proto_evt);
                    break;
                    
                case ACTION_NONE:
                    DBG_INFO("No action for this event");
                    break;
            }
        }
    }
}
