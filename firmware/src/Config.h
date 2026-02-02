#pragma once
#include <Arduino.h>

// --- POWER MANAGEMENT ---
#define PIN_VBUS_SENSE      10  // Voltage Divider (High = USB Connected)
#define PIN_LED_POWER       11  // Load Switch Gate (High = LEDs ON)

// --- MATRIX CONFIG ---
#define MATRIX_ROWS         3
#define MATRIX_COLS         3

// GPIOs (Update these to match your KiCad PCB!)
static const byte ROW_PINS[MATRIX_ROWS] = {5, 6, 7}; 
static const byte COL_PINS[MATRIX_COLS] = {1, 2, 3};

// --- ROTARY ENCODER ---
#define PIN_ENC_A           35
#define PIN_ENC_B           36
#define PIN_ENC_BTN         37

// --- LEDS ---
#define PIN_LEDS            12
#define NUM_LEDS            10

// --- SLEEP SETTINGS ---
#define SOFT_SLEEP_MS       60000   // 1 minute (Nap)
#define HARD_SLEEP_MS       1800000 // 30 mins (Deep Sleep)
