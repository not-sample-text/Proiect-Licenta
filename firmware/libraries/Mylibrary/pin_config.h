#pragma once

// OLED Display (I2C)
#define PIN_SDA 8
#define PIN_SCL 9
#define OLED_RST -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Rotary Encoder
#define PIN_ENC_CLK 10
#define PIN_ENC_DT 11
#define PIN_ENC_SW 12

// Key Matrix (3x4)
#define ROWS 4
#define COLS 3
#define PIN_ROW0 1
#define PIN_ROW1 2
#define PIN_ROW2 3
#define PIN_ROW3 7
#define PIN_COL0 4
#define PIN_COL1 5
#define PIN_COL2 6

// Physical Mode Switch
#define PIN_MODE_SWITCH 13
