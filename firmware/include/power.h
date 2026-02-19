/**
 * @file power.h
 * @brief Power management for the PROS3 Macropad.
 *
 * Handles light-sleep mode, peripheral power gating, and idle detection
 * to maximize battery life in BLE mode and reduce power draw in USB mode.
 */

#pragma once

#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_sleep.h>
#include <esp_pm.h>

// ── Configuration ───────────────────────────────────────────────
/**
 * Power management configuration — all thresholds in milliseconds.
 */
struct PowerConfig {
    uint32_t idle_timeout_ms;       // Time before entering light-sleep (default: 60000 = 1 min)
    uint32_t oled_dim_timeout_ms;   // Time before dimming OLED (default: 30000 = 30s)
    bool     sleep_enabled;         // Master switch for sleep mode
    
    PowerConfig()
        : idle_timeout_ms(60000)
        , oled_dim_timeout_ms(30000)
        , sleep_enabled(true)
    {}
};

extern PowerConfig g_power_config;

// ── Initialization ──────────────────────────────────────────────
/**
 * Initialize power management subsystem.
 * - Configures CPU frequency and power domains
 * - Disables unused peripherals (WiFi, BT radio if not needed)
 * - Sets up wake sources for light-sleep
 */
void power_init();

/**
 * Configure wake sources for light-sleep.
 * - All key matrix GPIOs (rows)
 * - Encoder GPIOs (CLK, DT, SW)
 * - VBUS sense (for USB plug detection)
 */
void power_configure_wake_sources();

// ── Activity Tracking ───────────────────────────────────────────
/**
 * Reset the idle timer — call whenever user input is detected.
 * (key press, encoder rotation, encoder button, etc.)
 */
void power_activity();

/**
 * Check if the device should enter sleep mode.
 * @return true if idle timeout expired and sleep is enabled
 */
bool power_should_sleep();

// ── Sleep Control ───────────────────────────────────────────────
/**
 * Enter light-sleep mode.
 * - Gates LED power via load switch
 * - Turns off OLED
 * - Puts ESP32-S3 into light-sleep
 * - Wakes on any configured GPIO interrupt
 */
void power_enter_sleep();

/**
 * Wake from sleep and restore peripherals.
 * Called automatically after wake — use to restore state.
 */
void power_wake();

// ── Peripheral Power Gating ─────────────────────────────────────
/**
 * Control the LED power rail via TPS22918 load switch.
 * @param enable true = power on, false = power off
 * 
 * Note: Assumes a GPIO controls the load switch enable pin.
 * If your hardware doesn't have this, these are no-ops.
 */
void power_set_leds(bool enable);

/**
 * Check if device is running on USB power (vs battery).
 * @return true if VBUS is present
 */
bool power_is_usb_powered();

// ── Utility ─────────────────────────────────────────────────────
/**
 * Get time since last activity (in milliseconds).
 */
uint32_t power_get_idle_time();
