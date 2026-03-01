/**
 * @file ble_hid.h
 * @brief Bluetooth Low Energy HID implementation for the PROS3 Macropad.
 *
 * Provides BLE HID keyboard and consumer control functionality using NimBLE.
 * Supports secure pairing, connection management, and mode switching.
 */

#pragma once

#include <Arduino.h>

// ── Connection State ────────────────────────────────────────────
enum BLEState : uint8_t {
    BLE_DISCONNECTED = 0,
    BLE_ADVERTISING,
    BLE_CONNECTED,
    BLE_PAIRED
};

// ── Initialization ──────────────────────────────────────────────
/**
 * Initialize BLE HID stack.
 * - Sets up NimBLE
 * - Configures HID services
 * - Starts advertising
 * 
 * @return true if initialized successfully
 */
bool ble_hid_init();

// ── State Management ────────────────────────────────────────────
/**
 * Check if BLE is enabled and should be used.
 * Reads the hardware BT select switch.
 */
bool ble_is_enabled();

/**
 * Get current BLE connection state.
 */
BLEState ble_get_state();

/**
 * Check if BLE is connected and ready to send reports.
 */
bool ble_is_ready();

// ── HID Reports ─────────────────────────────────────────────────
/**
 * Send a keyboard report over BLE.
 * @param keycode Primary keycode (0 for release)
 * @param modifiers Modifier mask
 * @param is_press true for press, false for release
 */
void ble_send_key(uint8_t keycode, uint8_t modifiers, bool is_press);

/**
 * Send a consumer control report over BLE.
 * @param usage_code HID consumer control usage code
 */
void ble_send_consumer(uint16_t usage_code);

/**
 * Send volume up command over BLE.
 */
void ble_volume_up();

/**
 * Send volume down command over BLE.
 */
void ble_volume_down();

/**
 * Send volume mute command over BLE.
 */
void ble_volume_mute();

// ── Connection Management ───────────────────────────────────────
/**
 * Start BLE advertising.
 * Called automatically on init, or manually after disconnect.
 */
void ble_start_advertising();

/**
 * Stop BLE advertising.
 * Useful when switching to USB mode.
 */
void ble_stop_advertising();

/**
 * Disconnect from current BLE host.
 */
void ble_disconnect();

/**
 * Clear pairing data and restart.
 * Forces re-pairing with host.
 */
void ble_clear_bonds();

// ── Task ────────────────────────────────────────────────────────
/**
 * BLE task handler.
 * Should be called regularly in main loop when BLE is enabled.
 */
void ble_hid_task();
