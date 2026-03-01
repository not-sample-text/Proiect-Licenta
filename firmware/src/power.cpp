/**
 * @file power.cpp
 * @brief Power management implementation.
 */

#include "power.h"
#include "pins.h"
#include "debug.h"
#include <esp_sleep.h>
#include <esp_pm.h>
#include <driver/gpio.h>

// ── State ───────────────────────────────────────────────────────
static unsigned long g_last_activity_ms = 0;
static PowerMode g_current_mode = POWER_MODE_ACTIVE;

// ── Initialization ──────────────────────────────────────────────
void power_init() {
    DBG_INFO("PWR", "Initializing power management...");
    
    // Start in active mode
    g_last_activity_ms = millis();
    g_current_mode = POWER_MODE_ACTIVE;
    
    // Configure battery voltage ADC pin
    pinMode(PIN_VBAT, INPUT);
    analogSetAttenuation(ADC_11db);  // 0-3.3V range
    
    // Configure encoder button as wake source for deep sleep
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_ENC_SW, 0); // Wake on LOW (button press)
    
    DBG_INFO("PWR", "Power management initialized (Active mode)");
}

// ── Activity Tracking ───────────────────────────────────────────
void power_activity() {
    g_last_activity_ms = millis();
    
    // Switch back to active mode if in light sleep
    if (g_current_mode == POWER_MODE_LIGHT_SLEEP) {
        power_set_active();
    }
}

unsigned long power_get_idle_time() {
    return millis() - g_last_activity_ms;
}

// ── Mode Control ────────────────────────────────────────────────
void power_set_active() {
    if (g_current_mode == POWER_MODE_ACTIVE) {
        return; // Already active
    }
    
    DBG_INFO("PWR", "Switching to ACTIVE mode (full speed)");
    
    // Full speed, no automatic sleep
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 240,
        .light_sleep_enable = false
    };
    esp_pm_configure(&pm_config);
    
    g_current_mode = POWER_MODE_ACTIVE;
}

void power_enable_light_sleep() {
    if (g_current_mode == POWER_MODE_LIGHT_SLEEP) {
        return; // Already in light sleep mode
    }
    
    DBG_INFO("PWR", "Enabling AUTO LIGHT SLEEP (CPU scales, BLE connected)");
    
    // Allow CPU to scale between 80-10 MHz and auto-sleep between BLE events
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = 80,
        .min_freq_mhz = 10,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);
    
    g_current_mode = POWER_MODE_LIGHT_SLEEP;
}

void power_enter_deep_sleep() {
    DBG_INFO("PWR", "Entering DEEP SLEEP (idle: %lu ms). Press encoder to wake.", 
             power_get_idle_time());
    
    // Flush serial output
    Serial.flush();
    delay(10);
    
    // Power off peripherals (OLED, RGB handled by their respective modules)
    // The encoder button wake source was configured in power_init()
    
    // Enter deep sleep - execution will not return
    // On wake, the ESP32-S3 will reboot completely
    esp_deep_sleep_start();
}

// ── Idle Check ──────────────────────────────────────────────────
void power_check_idle() {
    unsigned long idle_time = power_get_idle_time();
    
    // Deep sleep after 30s idle
    if (idle_time > DEEP_SLEEP_TIMEOUT) {
        DBG_INFO("PWR", "Idle timeout exceeded (%lu ms)", idle_time);
        power_enter_deep_sleep();
        // Execution does not return
    }
    
    // Auto light sleep after 10s idle
    if (idle_time > POWER_SAVE_TIMEOUT && g_current_mode == POWER_MODE_ACTIVE) {
        power_enable_light_sleep();
    }
}

// ── Battery Monitoring ──────────────────────────────────────────
#define VBAT_SAMPLES 4  // Average multiple samples for stability

uint16_t power_get_battery_mv() {
    // UM ProS3 has a voltage divider (2x) on VBAT pin
    // ADC reads 0-3.3V, battery is 3.0V-4.2V LiPo
    // Voltage divider: VBAT -> 100K -> ADC -> 100K -> GND
    // So ADC sees VBAT/2
    
    uint32_t adc_sum = 0;
    for (int i = 0; i < VBAT_SAMPLES; i++) {
        adc_sum += analogRead(PIN_VBAT);
        delayMicroseconds(100);
    }
    uint16_t adc_value = adc_sum / VBAT_SAMPLES;
    
    // ESP32-S3 ADC: 12-bit (0-4095), 0-3.3V range
    // Voltage at ADC = adc_value * 3.3 / 4095
    // Actual battery = voltage_at_adc * 2 (due to voltage divider)
    uint16_t voltage_mv = (adc_value * 3300 * 2) / 4095;
    
    return voltage_mv;
}

uint8_t power_get_battery_percent() {
    uint16_t voltage = power_get_battery_mv();
    
    // LiPo voltage curve (approximate):\n    // 4200mV = 100%
    // 3700mV = 50%
    // 3000mV = 0%
    
    if (voltage >= 4200) return 100;
    if (voltage <= 3000) return 0;
    
    // Linear approximation between 3.0V and 4.2V
    return (uint8_t)(((voltage - 3000) * 100) / 1200);
}

bool power_is_charging() {
    // Charging if USB is connected AND battery is not full
    if (!power_is_usb_connected()) {
        return false;
    }
    
    uint16_t voltage = power_get_battery_mv();
    return voltage < 4150;  // Not full (< 4.15V)
}

bool power_is_usb_connected() {
    return digitalRead(PIN_VBUS_SENSE) == HIGH;
}
