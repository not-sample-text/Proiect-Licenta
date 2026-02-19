/**
 * @file power.cpp
 * @brief Power management implementation for the PROS3 Macropad.
 */

#include "power.h"
#include "pins.h"
#include "debug.h"

// ── Global State ────────────────────────────────────────────────
PowerConfig g_power_config;

static uint32_t g_last_activity_ms = 0;
static bool     g_is_sleeping = false;

// We'll assume the TPS22918 load switch enable pin is connected to a GPIO.
// If your PCB doesn't have this, set to -1 and these functions become no-ops.
static const int LED_POWER_EN_PIN = -1;  // TODO: Update with actual GPIO if present

// ── Initialization ──────────────────────────────────────────────
void power_init() {
    DBG_INFO("Initializing power management...");
    
    // Disable WiFi and Bluetooth radios at startup to save power
    // (BLE will be enabled explicitly later if needed)
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    // WiFi is not initialized by default in Arduino, so nothing to disable
    #endif
    
    // Configure CPU frequency — ESP32-S3 can run at 240, 160, 80, 40, 20, 10 MHz
    // For a macropad, 80 MHz is plenty and saves significant power
    setCpuFrequencyMhz(80);
    DBG_INFO("  CPU frequency: %d MHz", getCpuFrequencyMhz());
    
    // Configure LED power control pin if present
    if (LED_POWER_EN_PIN >= 0) {
        pinMode(LED_POWER_EN_PIN, OUTPUT);
        digitalWrite(LED_POWER_EN_PIN, HIGH);  // LEDs on by default
        DBG_VERBOSE("  LED power control: GPIO %d", LED_POWER_EN_PIN);
    }
    
    // Configure wake sources
    power_configure_wake_sources();
    
    // Mark activity at boot
    g_last_activity_ms = millis();
    
    DBG_INFO("Power management initialized (%s)",
             g_power_config.sleep_enabled ? "sleep enabled" : "sleep disabled");
}

void power_configure_wake_sources() {
    DBG_VERBOSE("Configuring wake sources...");
    
    // Wake on any row pin going LOW (key press with active-low scan)
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        gpio_wakeup_enable((gpio_num_t)ROW_PINS[r], GPIO_INTR_LOW_LEVEL);
        DBG_VERBOSE("  Wake on ROW%d (GPIO %d)", r, ROW_PINS[r]);
    }
    
    // Wake on encoder transitions (either pin going LOW)
    gpio_wakeup_enable((gpio_num_t)PIN_ENC_CLK, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable((gpio_num_t)PIN_ENC_DT,  GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable((gpio_num_t)PIN_ENC_SW,  GPIO_INTR_LOW_LEVEL);
    DBG_VERBOSE("  Wake on encoder (GPIO %d, %d, %d)",
                PIN_ENC_CLK, PIN_ENC_DT, PIN_ENC_SW);
    
    // Wake on VBUS detection (USB plug-in)
    gpio_wakeup_enable((gpio_num_t)PIN_VBUS_SENSE, GPIO_INTR_HIGH_LEVEL);
    DBG_VERBOSE("  Wake on VBUS (GPIO %d)", PIN_VBUS_SENSE);
    
    // Enable GPIO wake
    esp_sleep_enable_gpio_wakeup();
}

// ── Activity Tracking ───────────────────────────────────────────
void power_activity() {
    g_last_activity_ms = millis();
    if (g_is_sleeping) {
        power_wake();
    }
}

bool power_should_sleep() {
    if (!g_power_config.sleep_enabled) {
        return false;
    }
    return power_get_idle_time() >= g_power_config.idle_timeout_ms;
}

uint32_t power_get_idle_time() {
    return millis() - g_last_activity_ms;
}

// ── Sleep Control ───────────────────────────────────────────────
void power_enter_sleep() {
    if (g_is_sleeping) {
        return;  // already sleeping
    }
    
    DBG_INFO("Entering light-sleep (idle: %lu ms)...", power_get_idle_time());
    
    // Gate LED power
    power_set_leds(false);
    
    // OLED will be turned off by the display subsystem before calling this
    // (we keep that logic separate to allow dimming without full sleep)
    
    g_is_sleeping = true;
    
    // Small delay to ensure serial output is flushed
    Serial.flush();
    delay(10);
    
    // Enter light-sleep — will wake on any configured GPIO
    esp_light_sleep_start();
    
    // Execution resumes here after wake
    power_wake();
}

void power_wake() {
    if (!g_is_sleeping) {
        return;  // wasn't sleeping
    }
    
    g_is_sleeping = false;
    g_last_activity_ms = millis();  // reset idle timer
    
    DBG_INFO("Woke from sleep");
    
    // Restore LED power
    power_set_leds(true);
    
    // OLED will be turned back on by the display subsystem
}

// ── Peripheral Power Gating ─────────────────────────────────────
void power_set_leds(bool enable) {
    if (LED_POWER_EN_PIN < 0) {
        return;  // no hardware control available
    }
    
    digitalWrite(LED_POWER_EN_PIN, enable ? HIGH : LOW);
    DBG_VERBOSE("LED power: %s", enable ? "ON" : "OFF");
}

bool power_is_usb_powered() {
    return digitalRead(PIN_VBUS_SENSE) == HIGH;
}
