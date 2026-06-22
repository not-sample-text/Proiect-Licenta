#pragma once

#include <Arduino.h>
#include <cstdint>
#include "config.h"
#include "handlers/debug_logger.h"

/**
 * MatrixHandler: Scans 4x3 keypad matrix with debouncing
 */
class MatrixHandler {
 public:
  MatrixHandler(DebugLogger& logger) : logger_(logger) {}

  void begin() {
    configureMatrixPins();
    clearStates();
  }

  void run() {
    scan();
  }

  /**
   * Get currently pressed key, or ' ' if none
   */
  char getLastPressedKey() const {
    return last_pressed_key_;
  }

  /**
   * Check if a key is currently stable/pressed
   */
  bool isKeyPressed(uint8_t row, uint8_t col) const {
    if (row >= kMatrixRowCount || col >= kMatrixColumnCount) return false;
    return stable_state_[row][col];
  }

 private:
  DebugLogger& logger_;
  char last_pressed_key_ = ' ';
  bool stable_state_[kMatrixRowCount][kMatrixColumnCount] = {};
  bool raw_state_[kMatrixRowCount][kMatrixColumnCount] = {};
  uint32_t last_change_ms_[kMatrixRowCount][kMatrixColumnCount] = {};

  void configureMatrixPins() {
    for (uint8_t row = 0; row < kMatrixRowCount; ++row) {
      pinMode(kMatrixRowPins[row], INPUT);
    }

    for (uint8_t col = 0; col < kMatrixColumnCount; ++col) {
      pinMode(kMatrixColPins[col], INPUT_PULLUP);
    }
  }

  void clearStates() {
    for (uint8_t r = 0; r < kMatrixRowCount; ++r) {
      for (uint8_t c = 0; c < kMatrixColumnCount; ++c) {
        stable_state_[r][c] = false;
        raw_state_[r][c] = false;
        last_change_ms_[r][c] = 0;
      }
    }
  }

  void scan() {
    uint32_t now = millis();
    bool changed = false;

    for (uint8_t row = 0; row < kMatrixRowCount; ++row) {
      pinMode(kMatrixRowPins[row], OUTPUT);
      digitalWrite(kMatrixRowPins[row], LOW);
      
      delayMicroseconds(100);

      for (uint8_t col = 0; col < kMatrixColumnCount; ++col) {
        bool pressed = digitalRead(kMatrixColPins[col]) == LOW;

        if (pressed != raw_state_[row][col]) {
          raw_state_[row][col] = pressed;
          last_change_ms_[row][col] = now;
        }

        if ((now - last_change_ms_[row][col]) >= kMatrixDebounceMs && 
            stable_state_[row][col] != raw_state_[row][col]) {
          stable_state_[row][col] = raw_state_[row][col];
          changed = true;
        }
      }

      pinMode(kMatrixRowPins[row], INPUT);

      for (uint8_t col = 0; col < kMatrixColumnCount; ++col) {
        pinMode(kMatrixColPins[col], OUTPUT);
        digitalWrite(kMatrixColPins[col], HIGH);
      }
      
      delayMicroseconds(10); 
      
      for (uint8_t col = 0; col < kMatrixColumnCount; ++col) {
        pinMode(kMatrixColPins[col], INPUT_PULLUP);
      }
    }

    // Update last pressed key
    if (changed) {
      char prev_key = last_pressed_key_;
      last_pressed_key_ = ' ';
      
      for (uint8_t r = 0; r < kMatrixRowCount; ++r) {
        for (uint8_t c = 0; c < kMatrixColumnCount; ++c) {
          if (stable_state_[r][c]) {
            last_pressed_key_ = kMatrixKeyMap[r][c];
            if (last_pressed_key_ != prev_key) {
              logger_.log("Matrix: Key '%c' Active", last_pressed_key_);
            }
          }
        }
      }
    }
  }
};
