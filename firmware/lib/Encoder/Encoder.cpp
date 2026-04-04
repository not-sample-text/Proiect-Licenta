/**
 * @file encoder.cpp
 * @brief Rotary encoder implementation.
 */

#include "Encoder.h"
#include "debug.h"
#include "Power.h"

// ── Configuration ───────────────────────────────────────────────
EncoderConfig g_encoder_config;

// ── Rotation State ──────────────────────────────────────────────
static volatile int32_t g_encoder_position = 0;
static volatile int8_t  g_encoder_step_count = 0;  // Sub-detent step counter
static volatile uint8_t g_encoder_last_state = 0;  // Last CLK|DT state (2 bits)

// ── Button State ────────────────────────────────────────────────
static bool     g_button_state = false;         // Current debounced state
static bool     g_button_raw = false;           // Raw state from last read
static uint32_t g_button_debounce_time = 0;     // Timestamp of last state change

// ── Encoder Mode ────────────────────────────────────────────────
static EncoderMode g_encoder_mode = ENCODER_MODE_VOLUME;  // Default to volume control

// ── State Machine Lookup Table ──────────────────────────────────
// Quadrature encoding state transition table
// Returns: +1 for CW, -1 for CCW, 0 for invalid/noise
static const int8_t ENCODER_TABLE[16] = {
    0,  // 0000: no change
    -1, // 0001: CCW step
    +1, // 0010: CW step
    0,  // 0011: invalid
    +1, // 0100: CW step
    0,  // 0101: no change
    0,  // 0110: invalid
    -1, // 0111: CCW step
    -1, // 1000: CCW step
    0,  // 1001: invalid
    0,  // 1010: no change
    +1, // 1011: CW step
    0,  // 1100: invalid
    +1, // 1101: CW step
    -1, // 1110: CCW step
    0   // 1111: no change
};

// ── Initialization ──────────────────────────────────────────────
void encoder_init() {
    DBG_INFO("ENC", "Initializing rotary encoder...");
    
    // Read initial state of CLK and DT
    uint8_t clk = digitalRead(PIN_ENC_CLK) ? 1 : 0;
    uint8_t dt  = digitalRead(PIN_ENC_DT)  ? 1 : 0;
    g_encoder_last_state = (clk << 1) | dt;
    
    // Attach interrupt to CLK pin (most encoders trigger on CLK transitions)
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), 
                    encoder_rotation_isr, 
                    CHANGE);
    
    // Also attach to DT for more responsive detection (optional but recommended)
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_DT), 
                    encoder_rotation_isr, 
                    CHANGE);
    
    DBG_VERBOSE("ENC", "  CLK interrupt: GPIO %d", PIN_ENC_CLK);
    DBG_VERBOSE("ENC", "  DT interrupt: GPIO %d", PIN_ENC_DT);
    DBG_VERBOSE("ENC", "  SW pin: GPIO %d (debounce: %d ms)", 
                PIN_ENC_SW, g_encoder_config.debounce_ms);
    
    DBG_INFO("ENC", "Encoder initialized (detent steps: %d)", g_encoder_config.detent_steps);
}

// ── Processing ──────────────────────────────────────────────────
void encoder_process() {
    if (!g_encoder_config.enabled) {
        return;
    }
    
    uint32_t now = millis();
    
    // Read button state
    bool is_pressed = (digitalRead(PIN_ENC_SW) == LOW);  // Active-low with pull-up
    
    // Check if raw state changed
    if (is_pressed != g_button_raw) {
        g_button_raw = is_pressed;
        g_button_debounce_time = now;
    }
    
    // Check if state has been stable for debounce period
    if (now - g_button_debounce_time >= g_encoder_config.debounce_ms) {
        // Check if debounced state changed
        if (g_button_raw != g_button_state) {
            g_button_state = g_button_raw;
            
            // Generate event
            InputEvent event;
            event.type = is_pressed ? EVENT_ENCODER_BUTTON_PRESS : EVENT_ENCODER_BUTTON_RELEASE;
            event.col = 0;  // Unused for encoder
            event.row = 0;  // Unused for encoder
            event.timestamp = now;
            
            if (!g_input_queue.enqueue(event)) {
                DBG_WARN("ENC", "Input queue full, dropped encoder button event");
            } else {
                DBG_VERBOSE("ENC", "Encoder button %s", is_pressed ? "PRESS" : "RELEASE");
            }
            
            // Reset power idle timer
            power_activity();
        }
    }
}

bool encoder_is_button_pressed() {
    return g_button_state;
}

int32_t encoder_get_position() {
    return g_encoder_position;
}

void encoder_reset_position() {
    g_encoder_position = 0;
    g_encoder_step_count = 0;
}

// ── Interrupt Handler ───────────────────────────────────────────
void IRAM_ATTR encoder_rotation_isr() {
    // Read current state of CLK and DT
    uint8_t clk = digitalRead(PIN_ENC_CLK) ? 1 : 0;
    uint8_t dt  = digitalRead(PIN_ENC_DT)  ? 1 : 0;
    uint8_t current_state = (clk << 1) | dt;
    
    // Build lookup index from previous and current state
    uint8_t index = (g_encoder_last_state << 2) | current_state;
    
    // Look up direction from state table
    int8_t direction = ENCODER_TABLE[index];
    
    if (direction != 0) {
        // Update step counter
        g_encoder_step_count += direction;
        
        // Check if we've completed a full detent
        if (abs(g_encoder_step_count) >= g_encoder_config.detent_steps) {
            // Determine final direction
            bool is_cw = (g_encoder_step_count > 0);
            
            // Reset step counter
            g_encoder_step_count = 0;
            
            // Update position
            g_encoder_position += (is_cw ? 1 : -1);
            
            // Generate event (directly from ISR for lowest latency)
            InputEvent event;
            event.type = is_cw ? EVENT_ENCODER_CW : EVENT_ENCODER_CCW;
            event.col = 0;
            event.row = 0;
            event.timestamp = millis();
            
            // Note: We don't log here (ISR context), just enqueue silently
            g_input_queue.enqueue(event);
            
            // Reset power idle timer
            // Note: power_activity() should be ISR-safe (just updates a timestamp)
            power_activity();
        }
    }
    
    // Save current state for next iteration
    g_encoder_last_state = current_state;
}

// ── Mode Control ────────────────────────────────────────────────
EncoderMode encoder_get_mode() {
    return g_encoder_mode;
}

void encoder_set_mode( EncoderMode mode) {
    if (g_encoder_mode != mode) {
        g_encoder_mode = mode;
        DBG_INFO("ENC", "Encoder mode: %s", 
                 mode == ENCODER_MODE_VOLUME ? "VOLUME" : "LAYER");
    }
}

EncoderMode encoder_toggle_mode() {
    EncoderMode new_mode = (g_encoder_mode == ENCODER_MODE_VOLUME) 
                           ? ENCODER_MODE_LAYER 
                           : ENCODER_MODE_VOLUME;
    encoder_set_mode(new_mode);
    return new_mode;
}
