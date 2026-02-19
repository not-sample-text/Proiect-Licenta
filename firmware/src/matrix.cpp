/**
 * @file matrix.cpp
 * @brief Key matrix scanner implementation.
 */

#include "matrix.h"
#include "debug.h"
#include "power.h"

// ── Configuration ───────────────────────────────────────────────
MatrixConfig g_matrix_config;

// ── State Tracking ──────────────────────────────────────────────
static uint8_t  g_key_state[MATRIX_ROWS][MATRIX_COLS];       // Current debounced state (1 = pressed)
static uint8_t  g_key_raw[MATRIX_ROWS][MATRIX_COLS];         // Raw state from last scan
static uint32_t g_key_debounce_time[MATRIX_ROWS][MATRIX_COLS]; // Timestamp of last state change

static volatile bool g_scan_needed = false;  // Flag set by ISR

// ── Initialization ──────────────────────────────────────────────
void matrix_init() {
    DBG_INFO("Initializing key matrix scanner...");
    
    // Clear state arrays
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            g_key_state[r][c] = 0;
            g_key_raw[r][c] = 0;
            g_key_debounce_time[r][c] = 0;
        }
    }
    
    // Attach interrupts to row pins (wake on any change)
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        attachInterrupt(digitalPinToInterrupt(ROW_PINS[r]), 
                        matrix_row_isr, 
                        CHANGE);
        DBG_VERBOSE("  Attached interrupt to ROW%d (GPIO %d)", r, ROW_PINS[r]);
    }
    
    DBG_INFO("Matrix scanner initialized (debounce: %d ms)", g_matrix_config.debounce_ms);
}

// ── Scanning ────────────────────────────────────────────────────
void matrix_scan() {
    if (!g_matrix_config.scan_enabled) {
        return;
    }
    
    uint32_t now = millis();
    
    // Scan each column
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        // Drive this column LOW
        digitalWrite(COL_PINS[c], LOW);
        
        // Small delay to let the signal settle (typically 1-2 µs is enough)
        delayMicroseconds(5);
        
        // Read all rows
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            bool is_pressed = (digitalRead(ROW_PINS[r]) == LOW);
            
            // Check if raw state changed
            if (is_pressed != g_key_raw[r][c]) {
                // State changed — start debounce timer
                g_key_raw[r][c] = is_pressed;
                g_key_debounce_time[r][c] = now;
            }
            
            // Check if state has been stable for debounce period
            if (now - g_key_debounce_time[r][c] >= g_matrix_config.debounce_ms) {
                // Check if debounced state changed
                if (g_key_raw[r][c] != g_key_state[r][c]) {
                    g_key_state[r][c] = g_key_raw[r][c];
                    
                    // Generate event
                    InputEvent event;
                    event.type = is_pressed ? EVENT_KEY_PRESS : EVENT_KEY_RELEASE;
                    event.col = c;
                    event.row = r;
                    event.timestamp = now;
                    
                    if (!g_input_queue.enqueue(event)) {
                        DBG_WARN("Input queue full, dropped key event C%dR%d", c, r);
                    } else {
                        DBG_VERBOSE("Key %s: C%dR%d", 
                                    is_pressed ? "PRESS" : "RELEASE", c, r);
                    }
                    
                    // Reset power idle timer on any key event
                    power_activity();
                }
            }
        }
        
        // Drive column back HIGH (idle state)
        digitalWrite(COL_PINS[c], HIGH);
    }
    
    // Clear ISR flag
    g_scan_needed = false;
}

bool matrix_is_pressed(uint8_t col, uint8_t row) {
    if (col >= MATRIX_COLS || row >= MATRIX_ROWS) {
        return false;
    }
    return g_key_state[row][col];
}

uint16_t matrix_get_state() {
    uint16_t state = 0;
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            if (g_key_state[r][c]) {
                state |= (1 << (r * MATRIX_COLS + c));
            }
        }
    }
    return state;
}

// ── Interrupt Handler ───────────────────────────────────────────
void IRAM_ATTR matrix_row_isr() {
    g_scan_needed = true;
}
