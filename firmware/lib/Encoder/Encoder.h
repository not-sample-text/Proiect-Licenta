/**
 * @file encoder.h
 * @brief Rotary encoder handler for the PROS3 Macropad.
 *
 * Implements a robust rotary encoder state machine with:
 * - Interrupt-driven rotation detection
 * - Direction tracking (CW/CCW)
 * - Button press/release detection with debouncing
 * - Detent filtering (prevents spurious events)
 * - Mode switching (volume control / layer cycling)
 *
 * Responsibilities:
 * - Monitor encoder CLK and DT pins via interrupt
 * - Decode rotation direction using quadrature encoding
 * - Detect button presses on the encoder switch
 * - Generate rotation and button events
 * - Push events to the global input queue
 */

#pragma once

#include <Arduino.h>
#include "pins.h"
#include "InputEvents.h"

// ── Encoder Mode ────────────────────────────────────────────────
enum EncoderMode {
    ENCODER_MODE_VOLUME,    // Rotation controls volume up/down
    ENCODER_MODE_LAYER      // Rotation cycles layers up/down
};

// ── Configuration ───────────────────────────────────────────────
struct EncoderConfig {
    uint8_t  debounce_ms;       // Button debounce time (default: 10 ms)
    uint8_t  detent_steps;      // Steps per detent (default: 4 for most encoders)
    bool     enabled;           // Master enable/disable
    
    EncoderConfig()
        : debounce_ms(10)
        , detent_steps(4)
        , enabled(true)
    {}
};

extern EncoderConfig g_encoder_config;

// ── Initialization ──────────────────────────────────────────────
/**
 * Initialize the rotary encoder.
 * - Configures interrupts on CLK and DT pins
 * - Initializes button debounce state
 * 
 * Note: Pins are already configured as INPUT by init_pins() in main.cpp.
 * This function only sets up the interrupt handlers.
 */
void encoder_init();

// ── Processing ──────────────────────────────────────────────────
/**
 * Process encoder state and button debouncing.
 * 
 * This should be called periodically from the main loop (e.g., every 1 ms).
 * It handles:
 * - Button debouncing
 * - Generating button press/release events
 * 
 * Rotation events are generated directly in the ISR (since they need
 * immediate response).
 */
void encoder_process();

/**
 * Get the current button state (debounced).
 * @return true if button is pressed, false if released
 */
bool encoder_is_button_pressed();

/**
 * Get the encoder position (accumulated steps since boot).
 * Can be positive or negative depending on rotation direction.
 */
int32_t encoder_get_position();

/**
 * Reset the encoder position to zero.
 */
void encoder_reset_position();

// ── Mode Control ────────────────────────────────────────────────
/**
 * Get the current encoder mode.
 * @return ENCODER_MODE_VOLUME or ENCODER_MODE_LAYER
 */
EncoderMode encoder_get_mode();

/**
 * Set the encoder mode.
 * @param mode New encoder mode
 */
void encoder_set_mode(EncoderMode mode);

/**
 * Toggle encoder mode between volume and layer.
 * @return New mode after toggle
 */
EncoderMode encoder_toggle_mode();

// ── Interrupt Handlers ──────────────────────────────────────────
/**
 * ISR for encoder rotation (attached to CLK pin).
 * Decodes direction and generates rotation events.
 */
void IRAM_ATTR encoder_rotation_isr();
