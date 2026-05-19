#include "BoardSupport.h"
#include "pins.h"
#include <Arduino.h>
#include <Wire.h>

void BoardSupport::begin() {
    // 1. Activate LED Power Rail (TPS22918)
    // GPIO 16 must be HIGH to enable the power switch for LEDs
    pinMode(PIN_LED_PWR_EN, OUTPUT);
    digitalWrite(PIN_LED_PWR_EN, HIGH);

    // 2. Initialize Serial for debugging
    Serial.begin(115200);

    // 3. Initialize I2C for OLED
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
}
