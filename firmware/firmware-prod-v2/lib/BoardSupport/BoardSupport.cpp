#include "BoardSupport.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>

void BoardSupport::begin() {
    // Isolate unused paths to prevent leakage current
    pinMode(kLegacyTpsEnablePin, OUTPUT);
    digitalWrite(kLegacyTpsEnablePin, LOW);
    pinMode(kLegacyRgbDataPin, OUTPUT);
    digitalWrite(kLegacyRgbDataPin, LOW);

    // Force external antenna selection
    pinMode(kAntennaPin, OUTPUT);
    digitalWrite(kAntennaPin, HIGH);

    // Initialize interactive lines
    pinMode(kBtSelectPin, INPUT_PULLUP);
    pinMode(kVbusSensePin, INPUT);

    Serial.begin(115200);
    Wire.begin(kOledSdaPin, kOledSclPin);
}

bool BoardSupport::isUsbConnected() {
    return digitalRead(kVbusSensePin) == HIGH;
}

bool BoardSupport::isBleSwitchActive() {
    // Inverted logic: HIGH (Up) is USB mode, LOW (Down) is BLE mode
    return digitalRead(kBtSelectPin) == LOW;
}

void BoardSupport::enterDeepSleep(bool shutdownMode) {
    if (shutdownMode) {
        // Wake up only when VBUS goes high (USB Plugged In)
        esp_sleep_enable_ext1_wakeup(1ULL << kVbusSensePin, ESP_EXT1_WAKEUP_ANY_HIGH);
    } else {
        // Inactivity wake up: Wake up on any row or encoder button low transition
        uint64_t mask = (1ULL << kEncoderSwPin);
        for (uint8_t i = 0; i < kMatrixRowCount; i++) {
            mask |= (1ULL << kMatrixRowPins[i]);
        }
        esp_sleep_enable_ext1_wakeup(mask, ESP_EXT1_WAKEUP_ANY_LOW);
    }
    
    esp_deep_sleep_start();
}
