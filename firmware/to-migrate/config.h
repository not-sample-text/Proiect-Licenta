#pragma once

#include <cstdint>

// ---------------------------------------------------------
// HARDWARE PIN CONFIGURATION
// ---------------------------------------------------------

// OLED Display (I2C)
constexpr uint8_t kOledAddress = 0x3C;
constexpr uint8_t kOledWidth = 128;
constexpr uint8_t kOledHeight = 32;
constexpr uint8_t kOledSdaPin = 8;
constexpr uint8_t kOledSclPin = 9;

// Buttons & Encoder
constexpr uint8_t kBtSelectPin = 34;
constexpr uint8_t kEncoderSwPin = 1;
constexpr uint8_t kEncoderClkPin = 2;
constexpr uint8_t kEncoderDtPin = 4;

// Power & Status
constexpr uint8_t kStatusLedPin = 18;
constexpr uint8_t kLdo2EnablePin = 17;
constexpr uint8_t kVbusSensePin = 21;

// Wireless
constexpr uint8_t kAntennaPin = 11;

// Fuel Gauge (I2C)
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
// MATRIX KEYPAD MAP
// ---------------------------------------------------------

constexpr char kMatrixKeyMap[kMatrixRowCount][kMatrixColumnCount] = {
  {'N', '#', '*'},
  {'7', '8', '9'},
  {'4', '5', '6'},
  {'1', '2', '3'},
};

// ---------------------------------------------------------
// DEBUG LOGGING
// ---------------------------------------------------------

constexpr uint8_t kMaxLogLines = 4;
constexpr uint8_t kMaxLogLength = 22; // 21 chars max at size 1 + null terminator

// ---------------------------------------------------------
// ENCODER DEBUG FLAGS
// ---------------------------------------------------------
// Set to true to enable detailed encoder state logging
constexpr bool kEncoderDebugLogging = true;
