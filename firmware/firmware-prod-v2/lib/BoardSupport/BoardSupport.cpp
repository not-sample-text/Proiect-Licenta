#include "BoardSupport.h"
#include "config.h"
#include "esp_sleep.h"
#include "esp_pm.h"
#include "driver/rtc_io.h"

void BoardSupport::begin() {
    pinMode(kBtSelectPin, INPUT_PULLUP);
    pinMode(kVbusSensePin, INPUT_PULLDOWN);

    // Dynamic Power Management & Auto Light Sleep
    // Drops CPU down to 40MHz during idle times to preserve battery
    #if CONFIG_PM_ENABLE
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 40,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);
    #endif
}

bool BoardSupport::isBleSwitchActive() {
    return digitalRead(kBtSelectPin) == LOW; 
}

bool BoardSupport::isUsbConnected() {
    // Pin 21 pulled down; goes high when 5V USB is plugged in
    return digitalRead(kVbusSensePin) == HIGH;
}

void BoardSupport::enterDeepSleep(bool hardShutdown) {
    if (hardShutdown) {
        // Hard Shutdown (4s Encoder Hold)
        // Ignored Matrix, wakes strictly when USB is plugged in
        // Pin 21 is RTC_GPIO10 on the S3, fully supporting EXT0 wake
        esp_sleep_enable_ext0_wakeup((gpio_num_t)kVbusSensePin, 1); 
    } else {
        // Timeout Sleep (Inactivity)
        // Wakes on ANY Matrix key press, Encoder button, or USB plug in
        
        uint64_t wakeMask = 0;
        
        // Add VBUS
        wakeMask |= (1ULL << kVbusSensePin);
        
        // Add Encoder Switch
        wakeMask |= (1ULL << kEncoderSwPin);
        
        // Add Matrix Rows (Assuming they pull HIGH upon press, adjust if they pull LOW)
        for(uint8_t i = 0; i < kMatrixRowCount; i++) {
            wakeMask |= (1ULL << kMatrixRowPins[i]);
        }
        
        // Add Matrix Cols
        for(uint8_t i = 0; i < kMatrixColumnCount; i++) {
            wakeMask |= (1ULL << kMatrixColPins[i]);
        }

        // Configure EXT1 vector to wake on any mapped pin going HIGH
        esp_sleep_enable_ext1_wakeup(wakeMask, ESP_EXT1_WAKEUP_ANY_HIGH);
    }

    esp_deep_sleep_start();
}
