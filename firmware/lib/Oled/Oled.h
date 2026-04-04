/**
 * @file oled.h
 * @brief OLED display driver for the PROS3 Macropad.
 *
 * Manages the SSD1306 128x32 OLED display via I2C.
 * Displays current layer, connection mode (USB/BLE), and status icons.
 * Implements efficient partial updates to minimize refresh time.
 */

#pragma once

#include <Arduino.h>

// ── Initialization ──────────────────────────────────────────────
/**
 * Initialize the OLED display.
 * - Sets up I2C communication
 * - Initializes U8g2 library
 * - Displays boot splash screen
 * 
 * @return true if display initialized successfully
 */
bool oled_init();

// ── Display Updates ─────────────────────────────────────────────
/**
 * Update the display with current state.
 * Only redraws when state changes to minimize CPU usage.
 * Should be called regularly in the main loop.
 */
void oled_update();

/**
 * Force a full display refresh.
 * Useful after wake from sleep or manual state changes.
 */
void oled_refresh();

// ── Content Updates ─────────────────────────────────────────────
/**
 * Set the current layer to display.
 * Display will update on next oled_update() call.
 */
void oled_set_layer(const char* layer_name);

/**
 * Set the connection mode indicator.
 * @param mode "USB" or "BLE"
 */
void oled_set_connection_mode(const char* mode);

/**
 * Set the last pressed key label to display.
 * @param label Key label from config (e.g., "New Tab")
 */
void oled_set_last_key(const char* label);

/**
 * Set the encoder mode to display.
 * @param mode "VOLUME" or "LAYER"
 */
void oled_set_encoder_mode(const char* mode);

/**
 * Show a temporary status message.
 * Message will auto-clear after timeout.
 * @param message Message to display (max 21 chars)
 * @param timeout_ms How long to show (0 = until cleared)
 */
void oled_show_status(const char* message, uint32_t timeout_ms = 2000);

/**
 * Clear any temporary status message.
 */
void oled_clear_status();

// ── Power Management ────────────────────────────────────────────
/**
 * Turn off the display (sleep mode).
 * Used when entering power saving mode.
 */
void oled_sleep();

/**
 * Turn on the display.
 * Used when waking from power saving mode.
 */
void oled_wake();

/**
 * Dim the display to low brightness.
 * Used during idle periods before full sleep.
 */
void oled_dim();

/**
 * Set display to normal brightness.
 */
void oled_bright();
