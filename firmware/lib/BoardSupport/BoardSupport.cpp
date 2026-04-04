#include "BoardSupport.h"
#include <Arduino.h>
#include "pins.h"
#include "debug.h"

void BoardSupport::begin() {
    DBG_INFO("MAIN", "Initializing GPIOs...");

    pinMode(PIN_LED_STATUS, OUTPUT);
    digitalWrite(PIN_LED_STATUS, LOW);

    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        pinMode(COL_PINS[c], OUTPUT);
        digitalWrite(COL_PINS[c], HIGH);
    }
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        pinMode(ROW_PINS[r], INPUT);
    }
    DBG_VERBOSE("MAIN", "  Key matrix: %d cols x %d rows", MATRIX_COLS, MATRIX_ROWS);

    pinMode(PIN_ENC_CLK, INPUT);
    pinMode(PIN_ENC_DT, INPUT);
    pinMode(PIN_ENC_SW, INPUT);
    DBG_VERBOSE("MAIN", "  Encoder: CLK=%d  DT=%d  SW=%d", PIN_ENC_CLK, PIN_ENC_DT, PIN_ENC_SW);

    DBG_VERBOSE("MAIN", "  OLED I2C: SDA=%d  SCL=%d", PIN_OLED_SDA, PIN_OLED_SCL);

    pinMode(PIN_RGB_LEDS, OUTPUT);
    digitalWrite(PIN_RGB_LEDS, LOW);
    DBG_VERBOSE("MAIN", "  RGB data pin: %d  (%d LEDs)", PIN_RGB_LEDS, RGB_LED_COUNT);

    pinMode(PIN_BT_SELECT, INPUT);
    DBG_VERBOSE("MAIN", "  BT select: %d", PIN_BT_SELECT);

    pinMode(PIN_VBUS_SENSE, INPUT);
    DBG_VERBOSE("MAIN", "  VBUS sense: %d", PIN_VBUS_SENSE);

    DBG_INFO("MAIN", "GPIOs initialised OK");
}

void BoardSupport::boot_blink() {
    for (uint8_t i = 0; i < 3; i++) {
        digitalWrite(PIN_LED_STATUS, HIGH);
        delay(100);
        digitalWrite(PIN_LED_STATUS, LOW);
        delay(100);
    }
}
