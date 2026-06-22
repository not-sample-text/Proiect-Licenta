#pragma once

#include <cstdint>

// ---------------------------------------------------------
// HARDWARE PIN CONFIGURATION
// ---------------------------------------------------------

// OLED Display (I2C)
constexpr uint8_t kOledAddress = 0x3C;
constexpr uint8_t kOledWidth = 128; 
constexpr uint8_t kOledHeight = 32;
constexpr uint8_t kOledSdaPin = 8; // SDA pin for I2C communication
constexpr uint8_t kOledSclPin = 9; // SCL pin for I2C communication

// Buttons & Encoder
constexpr uint8_t kBtSelectPin = 34; // SPDT switch for selecting between wired and bluetooth mode (ble) -> low wired; high ble
constexpr uint8_t kEncoderSwPin = 1; // Encoder button pin
constexpr uint8_t kEncoderClkPin = 2; // Encoder clock pin
constexpr uint8_t kEncoderDtPin = 4; // Encoder data pin

// Power & Status
constexpr uint8_t kStatusLedPin = 18; // Status LED pin (onboard pros3)
constexpr uint8_t kLdo2EnablePin = 17; // LDO2 enable pin (controls power to the status led)
constexpr uint8_t kVbusSensePin = 21; // VBUS sense pin (detects if USB is connected) -> added by me, not the onboard 5v detector

// Wireless
constexpr uint8_t kAntennaPin = 11; // Antenna pin for wireless communication (low = internal antenna, high = external antenna, always use high)

// Fuel Gauge (I2C) -> onboard the pro s3
constexpr uint8_t kFuelGaugeAddress = 0x36;
constexpr uint8_t kFuelGaugeVcellRegister = 0x02;
constexpr uint8_t kFuelGaugeSocRegister = 0x04;

// Matrix Keypad
constexpr uint8_t kMatrixRowCount = 4;
constexpr uint8_t kMatrixColumnCount = 3;
constexpr uint8_t kMatrixColPins[kMatrixColumnCount] = {15, 37, 35};
constexpr uint8_t kMatrixRowPins[kMatrixRowCount] = {12, 13, 14, 5};
constexpr uint32_t kMatrixDebounceMs = 5;

// ---------------------------------------------------------
// TIMING & BEHAVIOR
// ---------------------------------------------------------

constexpr uint32_t kInitDelayMs = 1000;
constexpr uint32_t kOledInitDelayMs = 100;
constexpr uint32_t kBootFlashDurationMs = 2000;
constexpr uint32_t kStatusLedBlinkMs = 250;
constexpr uint32_t kEncoderHoldDurationMs = 2000;
constexpr uint32_t kBleJustConnectedDurationMs = 7500;

// ---------------------------------------------------------
// MATRIX KEYPAD MAP -> only for testing, final product uses hid
// ---------------------------------------------------------

constexpr char kMatrixKeyMap[kMatrixRowCount][kMatrixColumnCount] = {
  {'N', '#', '*'},
  {'7', '8', '9'},
  {'4', '5', '6'},
  {'1', '2', '3'},
};

// ---------------------------------------------------------
// DEBUG LOGGING -> also only for testing, final product logs to serial
// ---------------------------------------------------------

constexpr uint8_t kMaxLogLines = 4;
constexpr uint8_t kMaxLogLength = 22; // 21 chars max at size 1 + null terminator

// ---------------------------------------------------------
// ENCODER DEBUG FLAGS
// ---------------------------------------------------------
// Set to true to enable detailed encoder state logging
constexpr bool kEncoderDebugLogging = true;
