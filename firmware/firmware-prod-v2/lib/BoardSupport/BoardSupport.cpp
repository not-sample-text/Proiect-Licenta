#include "BoardSupport.h"
#include "config.h"
#include <Wire.h>

void BoardSupport::begin() {
    // Isolate the unused TPS22918 power line by forcing it flat LOW
    pinMode(kLedPwrEnPin, OUTPUT);
    digitalWrite(kLedPwrEnPin, LOW);

    // Route RF pathways to use the external antenna explicitly
    pinMode(kAntennaPin, OUTPUT);
    digitalWrite(kAntennaPin, HIGH);

    // Enable LDO2 to power the onboard status pixel
    pinMode(kLdo2EnablePin, OUTPUT);
    digitalWrite(kLdo2EnablePin, HIGH);

    pinMode(kBtSelectPin, INPUT_PULLUP);
    pinMode(kVbusSensePin, INPUT);

    Serial.begin(115200);
    Wire.begin(kOledSdaPin, kOledSclPin);
}

bool BoardSupport::isUsbConnected() {
    return digitalRead(kVbusSensePin) == HIGH;
}

bool BoardSupport::isBleSwitchActive() {
    return digitalRead(kBtSelectPin) == HIGH;
}

void BoardSupport::enterDeepSleep(bool shutdownMode) {
    digitalWrite(kLedPwrEnPin, LOW);
    
    if (shutdownMode) {
        // Wake source for manual shutdown is strictly USB attachment going high
        esp_deep_sleep_enable_gpio_wakeup_bitmask(1ULL << kVbusSensePin, ESP_GPIO_WAKEUP_GPIO_HIGH);
    } else {
        configureInactivityWakeup();
    }
    esp_deep_sleep_start();
}

void BoardSupport::configureInactivityWakeup() {
    // Enable any key press or encoder interaction to wake from idle deep sleep
    for (uint8_t pin : kMatrixRowPins) {
        esp_deep_sleep_enable_gpio_wakeup_bitmask(1ULL << pin, ESP_GPIO_WAKEUP_GPIO_LOW);
    }
    esp_deep_sleep_enable_gpio_wakeup_bitmask(1ULL << kEncoderSwPin, ESP_GPIO_WAKEUP_GPIO_LOW);
}
