#pragma once

#include <Arduino.h>

// ── Power Management ──────────────────────────────────────────
#define PIN_LED_PWR_EN  16  // TPS22918 EN: HIGH=LED rail ON, LOW=LED rail OFF
#define PIN_VBAT        10  // Battery voltage sense (ADC)
#define PIN_VBUS_SENSE  21  // USB 5V detection

// ── Key Matrix (3 columns × 4 rows) ────────────────────────────
// #define PIN_COL0        15
#define PIN_COL1        37
#define PIN_COL2        35

#define PIN_ROW0        12
#define PIN_ROW1        13
#define PIN_ROW2        14
#define PIN_ROW3         5

static constexpr uint8_t MATRIX_COLS = 3;
static constexpr uint8_t MATRIX_ROWS = 4;
static constexpr uint8_t COLS_PINS[MATRIX_COLS] = { PIN_COL0, PIN_COL1, PIN_COL2 };
static constexpr uint8_t ROWS_PINS[MATRIX_ROWS] = { PIN_ROW0, PIN_ROW1, PIN_ROW2, PIN_ROW3 };

// ── Rotary Encoder (EC11) ──────────────────────────────────────
#define PIN_ENC_CLK      2
#define PIN_ENC_DT       4
#define PIN_ENC_SW       1

// ── OLED Display (I2C) ─────────────────────────────────────────
#define PIN_I2C_SDA      8
#define PIN_I2C_SCL      9

// ── Addressable LEDs (SK6812) ──────────────────────────────────
#define PIN_RGB_DATA     7
#define RGB_COUNT       10

// ── User Input / Config ────────────────────────────────────────
#define PIN_BT_SELECT   34  // BLE vs USB mode switch
