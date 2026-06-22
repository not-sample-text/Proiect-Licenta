#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <cstdint>
#include "config.h"
#include "handlers/debug_logger.h"

/**
 * BatteryManager: Reads battery percentage and voltage from MAX17048 fuel gauge
 */
class BatteryManager {
 public:
  BatteryManager(DebugLogger& logger) : logger_(logger) {}

  void begin() {
    // Battery manager uses Wire which is initialized elsewhere
  }

  void run() {
    // Battery reads are performed on-demand by display
  }

  /**
   * Read battery percentage (0-100)
   */
  bool getPercent(uint8_t& percent) const {
    uint16_t socRaw = 0;
    if (!read16(kFuelGaugeSocRegister, socRaw)) {
      return false;
    }

    uint16_t whole = socRaw >> 8;
    uint8_t fractional = static_cast<uint8_t>(socRaw & 0xFF);
    uint16_t rounded = whole + ((fractional >= 128) ? 1 : 0);
    if (rounded > 100) {
      rounded = 100;
    }

    percent = static_cast<uint8_t>(rounded);
    return true;
  }

  /**
   * Read battery voltage in millivolts
   */
  bool getVoltageMv(uint16_t& voltageMv) const {
    uint16_t vcellRaw = 0;
    if (!read16(kFuelGaugeVcellRegister, vcellRaw)) {
      return false;
    }

    uint64_t millivolts = (static_cast<uint64_t>(vcellRaw) * 78125ULL + 500000ULL) / 1000000ULL;
    voltageMv = static_cast<uint16_t>(millivolts);
    return true;
  }

 private:
  DebugLogger& logger_;

  /**
   * Read 16-bit value from fuel gauge register
   */
  bool read16(uint8_t reg, uint16_t& value) const {
    Wire.beginTransmission(kFuelGaugeAddress);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
      return false;
    }

    if (Wire.requestFrom(static_cast<int>(kFuelGaugeAddress), 2) != 2) {
      return false;
    }

    uint8_t msb = static_cast<uint8_t>(Wire.read());
    uint8_t lsb = static_cast<uint8_t>(Wire.read());
    value = static_cast<uint16_t>((static_cast<uint16_t>(msb) << 8) | lsb);
    return true;
  }
};
