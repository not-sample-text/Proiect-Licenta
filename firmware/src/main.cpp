#include <Arduino.h>
#include "Config.h"
#include "managers/InputManager.h"
#include "managers/LightingManager.h" // Assuming you have this or will stub it

// Globals
InputManager inputMgr;
unsigned long lastActivityTime = 0;

// Deep Sleep Helper
void enterDeepSleep(bool allowEncoderWake) {
    Serial.println("Sleep...");
    Serial.flush();

    // 1. Cut Power to Peripherals
    digitalWrite(PIN_LED_POWER, LOW); // Load Switch OFF
    
    // 2. Configure Wake Sources
    // Wake on VBUS High (USB Plugged in)
    // GPIO 10 is in RTC domain on S3, so ext1 works
    esp_sleep_enable_ext1_wakeup((1ULL << PIN_VBUS_SENSE), ESP_EXT1_WAKEUP_ANY_HIGH);

    // Wake on Encoder (if allowed)
    if (allowEncoderWake) {
        esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_ENC_BTN, 0); // Wake on Low
    }

    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);
    
    // Hardware Init
    pinMode(PIN_VBUS_SENSE, INPUT);
    pinMode(PIN_LED_POWER, OUTPUT);
    
    // Turn ON LEDs
    digitalWrite(PIN_LED_POWER, HIGH);
    delay(10); 
    
    inputMgr.begin();
    
    lastActivityTime = millis();
    
    // Check why we woke up
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
         Serial.println("Woke via USB!");
    }
}

void loop() {
    // 1. Read Inputs
    String msg = inputMgr.getEvent();
    
    if (msg != "") {
        // Send to Python Host
        Serial.println(msg); 
    }

    // 2. Update Timers
    if (inputMgr.isActivityDetected()) {
        lastActivityTime = millis();
    }

    // 3. Sleep Logic
    unsigned long idleTime = millis() - lastActivityTime;
    bool usbConnected = digitalRead(PIN_VBUS_SENSE);

    // Hard Sleep Check (Battery Only, Timeout)
    if (!usbConnected && idleTime > HARD_SLEEP_MS) {
        enterDeepSleep(false); // Only wake on USB
    }
    // Soft Sleep Check (Idle for 1 min)
    else if (idleTime > SOFT_SLEEP_MS) {
        enterDeepSleep(true); // Wake on Encoder or USB
    }
    
    delay(10); // Stability
}
