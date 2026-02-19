/**
 * @file matrix.h
 * @brief Key matrix scanner for the PROS3 Macropad.
 *
 * Implements a 3×4 key matrix scanner with:
 * - Hardware debouncing (time-based)
 * - Interrupt-driven scanning (wake on any row change)
 * - Low latency (target: ≤5 ms from press to event)
 *
 * Scan method: Active-low column scan
 * - Columns are OUTPUT, driven LOW one at a time
 * - Rows are INPUT with external pull-ups
 * - Key press pulls row LOW
 *
 * Responsibilities:
 * - Scan the key matrix periodically
 * - Debounce key state changes
 * - Generate press/release events
 * - Push events to the global input queue
 */

#pragma once

#include <Arduino.h>
#include "pins.h"
#include "input_events.h"

// ── Configuration ───────────────────────────────────────────────
struct MatrixConfig {
    uint8_t  debounce_ms;       // Debounce time (default: 5 ms)
    bool     scan_enabled;      // Master enable/disable
    
    MatrixConfig()
        : debounce_ms(5)
        , scan_enabled(true)
    {}
};

extern MatrixConfig g_matrix_config;

// ── Initialization ──────────────────────────────────────────────
/**
 * Initialize the key matrix scanner.
 * - Configures GPIO interrupt on row pins
 * - Initializes debounce state
 * 
 * Note: Pins are already configured as INPUT/OUTPUT by init_pins() in main.cpp.
 * This function only sets up the scanning logic.
 */
void matrix_init();

// ── Scanning ────────────────────────────────────────────────────
/**
 * Scan the key matrix once and update debounce state.
 * 
 * This should be called periodically from the main loop (e.g., every 1 ms)
 * to maintain low latency. Debouncing is handled internally.
 * 
 * When a key state change is confirmed (after debounce):
 * - A press/release event is pushed to g_input_queue
 * - power_activity() is called to reset idle timer
 */
void matrix_scan();

/**
 * Get the current raw state of a specific key (debounced).
 * @param col Column index (0-2)
 * @param row Row index (0-3)
 * @return true if key is pressed, false if released
 */
bool matrix_is_pressed(uint8_t col, uint8_t row);

/**
 * Get a bitmask of all currently pressed keys (debounced).
 * Bit layout: [row3_col2, row3_col1, row3_col0, ..., row0_col2, row0_col1, row0_col0]
 * @return 12-bit value where bit N = key at (col=N%3, row=N/3)
 */
uint16_t matrix_get_state();

// ── Interrupt Handler ───────────────────────────────────────────
/**
 * ISR for row pin changes — sets a flag to wake the main loop.
 * Should be attached to all row pins via attachInterrupt().
 */
void IRAM_ATTR matrix_row_isr();
