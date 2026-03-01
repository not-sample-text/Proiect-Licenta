/**
 * @file rgb.h
 * @brief RGB underglow driver for the PROS3 Macropad.
 *
 * Controls 10x SK6812MINI RGB LEDs using the Adafruit NeoPixel library.
 * Supports multiple effects: solid color, breathing, rainbow cycle.
 * Settings are loaded from config.json.
 */

#pragma once

#include <Arduino.h>

// ── RGB Modes ───────────────────────────────────────────────────
enum RGBMode : uint8_t {
    RGB_MODE_OFF = 0,
    RGB_MODE_SOLID,
    RGB_MODE_BREATHING,
    RGB_MODE_RAINBOW,
    RGB_MODE_CYCLE,
    RGB_MODE_COUNT
};

// ── Initialization ──────────────────────────────────────────────
/**
 * Initialize RGB LED strip.
 * - Configures data pin
 * - Initializes NeoPixel library
 * - Loads settings from config
 * 
 * @return true if initialized successfully
 */
bool rgb_init();

// ── Updates ─────────────────────────────────────────────────────
/**
 * Update RGB animation.
 * Should be called regularly in the main loop (every ~20ms).
 */
void rgb_update();

// ── Settings ────────────────────────────────────────────────────
/**
 * Set RGB mode.
 * Changes are applied immediately.
 */
void rgb_set_mode(RGBMode mode);

/**
 * Set solid color (for SOLID and BREATHING modes).
 * @param r Red (0-255)
 * @param g Green (0-255)
 * @param b Blue (0-255)
 */
void rgb_set_color(uint8_t r, uint8_t g, uint8_t b);

/**
 * Set brightness (0-255).
 * Capped at firmware max to protect power consumption.
 */
void rgb_set_brightness(uint8_t brightness);

/**
 * Set animation speed (0-255).
 * Higher = faster animation.
 */
void rgb_set_speed(uint8_t speed);

// ── Power Management ────────────────────────────────────────────
/**
 * Turn off all LEDs (power saving).
 */
void rgb_off();

/**
 * Restore previous state after wake.
 */
void rgb_restore();
