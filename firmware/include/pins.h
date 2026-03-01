/**
 * @file pins.h
 * @brief GPIO pin definitions for the PROS3 Macropad.
 *
 * All pin assignments match the bottom-board schematic and the
 * UM ProS3 (ESP32-S3) module pinout.
 */

#pragma once

// ── Key Matrix (3 columns × 4 rows) ────────────────────────────
#define PIN_COL0        36
#define PIN_COL1        37
#define PIN_COL2        35

#define PIN_ROW0        12
#define PIN_ROW1        13
#define PIN_ROW2        14
#define PIN_ROW3         5

#define MATRIX_COLS      3
#define MATRIX_ROWS      4

static constexpr uint8_t COL_PINS[MATRIX_COLS] = { PIN_COL0, PIN_COL1, PIN_COL2 };
static constexpr uint8_t ROW_PINS[MATRIX_ROWS] = { PIN_ROW0, PIN_ROW1, PIN_ROW2, PIN_ROW3 };

// ── Rotary Encoder ──────────────────────────────────────────────
#define PIN_ENC_CLK      2   // Encoder A
#define PIN_ENC_DT       4   // Encoder B
#define PIN_ENC_SW       1   // Encoder push-button

// ── OLED Display (I²C) ─────────────────────────────────────────
#define PIN_OLED_SDA     8
#define PIN_OLED_SCL     9

// ── Addressable LEDs (SK6812MINI × 10) ─────────────────────────
#define PIN_RGB_LEDS     7
#define RGB_LED_COUNT   10

// ── Mode Selection ──────────────────────────────────────────────
#define PIN_BT_SELECT   34   // Slide switch: BLE / USB mode

// ── Sensing / Status ────────────────────────────────────────────
#define PIN_VBUS_SENSE  21   // USB 5 V detection (RTC wake-capable)
#define PIN_VBAT        10   // Battery voltage sense (ADC with voltage divider)
#define PIN_LED_STATUS  16   // On-board status LED
