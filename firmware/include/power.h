/**
 * @file power.h
 * @brief Power management for the PROS3 Macropad.
 *
 * Three-tier power system:
 * 1. Active (full speed) - During active use
 * 2. Auto light sleep - CPU scales down, BLE stays connected
 * 3. Deep sleep - Last resort, BLE disconnects, full reboot on wake
 */

#pragma once

#include <Arduino.h>

// ── Power Modes ─────────────────────────────────────────────────
enum PowerMode {
    POWER_MODE_ACTIVE,      // Full speed
    POWER_MODE_LIGHT_SLEEP, // CPU scales down, auto light sleep
    POWER_MODE_DEEP_SLEEP   // Everything off except RTC
};

// ── Configuration ───────────────────────────────────────────────
#define POWER_SAVE_TIMEOUT       10000   // 10s no activity → auto light sleep
#define DEEP_SLEEP_TIMEOUT       30000   // 30s no activity → deep sleep
#define DISCONNECTED_TIMEOUT     120000  // 2 min disconnected → deep sleep

// ── Initialization ──────────────────────────────────────────────
/**
 * Initialize power management system.
 * Sets up wake sources and initial power mode.
 */
void power_init();

// ── Activity Tracking ───────────────────────────────────────────
/**
 * Record user activity (key press, encoder turn, etc.).
 * Resets idle timer and switches to active mode if needed.
 */
void power_activity();

/**
 * Get milliseconds since last activity.
 */
unsigned long power_get_idle_time();

// ── Mode Control ────────────────────────────────────────────────
/**
 * Enable active mode (full speed).
 * Called automatically by power_activity().
 */
void power_set_active();

/**
 * Enable automatic light sleep mode.
 * CPU scales between max/min freq, BLE stays connected.
 */
void power_enable_light_sleep();

/**
 * Enter deep sleep.
 * Everything off except RTC. Full reboot on wake.
 * Wake source: encoder button.
 */
void power_enter_deep_sleep();

// ── Idle Check ──────────────────────────────────────────────────
/**
 * Check idle timeout and transition power modes as needed.
 * Should be called regularly from main loop.
 */
void power_check_idle();

// ── Battery Monitoring ──────────────────────────────────────────
/**
 * Read battery voltage in millivolts.
 * Returns 0 if battery monitoring is not available.
 */
uint16_t power_get_battery_mv();

/**
 * Get battery charge level as percentage (0-100).
 * Returns 0 if battery monitoring is not available.
 */
uint8_t power_get_battery_percent();

/**
 * Check if device is currently charging.
 * @return true if VBUS is present and battery is not full
 */
bool power_is_charging();

/**
 * Check if USB power is connected.
 * @return true if VBUS is detected
 */
bool power_is_usb_connected();
