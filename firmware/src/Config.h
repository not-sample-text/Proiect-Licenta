#pragma once
#include <Arduino.h>

// --- SYSTEM ---
#define SERIAL_BAUD_RATE 115200
#define DEVICE_NAME "ESP32-S3 Macropad"
#define MANUFACTURER_NAME "Bachelor Thesis"

// --- KEY MATRIX ---
// Rows are usually driven (Output)
const uint8_t ROW_PINS[4] = { 1, 2, 4, 5 }; 
// Columns are usually read (Input with Pullup)
const uint8_t COL_PINS[3] = { 37, 36, 34 }; 

// --- ROTARY ENCODER ---
#define ENC_PIN_A   15 // DT
#define ENC_PIN_B   13 // CLK
#define ENC_BUTTON  12 // SW

// --- PERIPHERALS ---
#define OLED_SDA    8
#define OLED_SCL    9
#define LED_PIN     7
#define NUM_LEDS    10

// --- SWITCHES ---
#define BT_SELECT_PIN 35 // Slide switch for Mode

// --- FILESYSTEM ---
#define CONFIG_FILE_PATH "/config.json"

// --- DATA STRUCTURES ---

// Represents a single key's function
struct KeyDefinition {
    String type;   // "KEY", "SHORTCUT", "SCRIPT", "APP"
    String label;  // "Copy", "Chrome"
    String value;  // "Ctrl+C", "chrome.exe"
};

// Represents the Global Lighting Settings
struct LightingConfig {
    uint8_t mode;       // 0=Solid, 1=Rainbow, etc.
    uint32_t color;     // Hex color (0xRRGGBB)
    uint8_t brightness; // 0-255
    uint8_t speed;      // 1-20
};

// The Master Configuration Object
struct SystemState {
    // 4 Layers, 4 Rows, 3 Columns
    KeyDefinition layers[4][4][3]; 
    LightingConfig lighting;
};
