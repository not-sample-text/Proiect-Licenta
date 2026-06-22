#pragma once

#include <Adafruit_NeoPixel.h>
#include <cstdint>
#include "config.h"

/**
 * BLE state enumeration
 */
enum class BleState {
  kPairing,
  kJustConnected,
  kConnected,
  kDisconnected
};

/**
 * StatusLedManager: Controls WS2812B RGB LED based on system state
 */
class StatusLedManager {
 public:
  StatusLedManager() : led_(1, kStatusLedPin, NEO_GRB + NEO_KHZ800) {}

  void begin() {
    led_.begin();
    led_.setBrightness(255);
    led_.show();
  }

  void run(uint32_t now, BleState bleState, uint8_t batteryPercent) {
    // Skip LED updates during early boot
    if (bootState_ < 1) {
      return;
    }

    static uint32_t lastBlinkMs = 0;
    static bool blinkState = false;
    
    if ((now - lastBlinkMs) >= kStatusLedBlinkMs) {
      blinkState = !blinkState;
      lastBlinkMs = now;
    }

    // Handle BLE pairing (blink blue)
    if (bleState == BleState::kPairing) {
      if (blinkState) {
        led_.setPixelColor(0, led_.Color(0, 0, 255));
      } else {
        led_.setPixelColor(0, led_.Color(0, 0, 0));
      }
      led_.show();
      return;
    }

    // Handle just connected (solid blue)
    if (bleState == BleState::kJustConnected) {
      led_.setPixelColor(0, led_.Color(0, 0, 255));
      led_.show();
      return;
    }

    // Handle low battery (blink red)
    if (batteryPercent <= 20) {
      bool slowBlink = ((now / 500) % 2) == 0;
      if (slowBlink) {
        led_.setPixelColor(0, led_.Color(255, 0, 0));
      } else {
        led_.setPixelColor(0, led_.Color(0, 0, 0));
      }
      led_.show();
      return;
    }

    // Color gradient based on battery level
    uint8_t red = 0;
    uint8_t green = 0;

    if (batteryPercent >= 60) {
      red = map(batteryPercent, 60, 100, 255, 0);
      green = 255;
    } else {
      red = 255;
      green = map(batteryPercent, 21, 59, 0, 255);
    }

    led_.setPixelColor(0, led_.Color(red, green, 0));
    led_.show();
  }

  void setBootState(uint8_t state) {
    bootState_ = state;
  }

 private:
  Adafruit_NeoPixel led_;
  uint8_t bootState_ = 0;
};
