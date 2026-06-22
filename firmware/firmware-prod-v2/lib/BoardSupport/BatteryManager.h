#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

class BatteryManager {
public:
    static bool getPercent(uint8_t& percent) {
        uint16_t socRaw = 0;
        if (!read16(kFuelGaugeSocRegister, socRaw)) return false;
        uint16_t whole = socRaw >> 8;
        uint8_t fractional = static_cast<uint8_t>(socRaw & 0xFF);
        uint16_t rounded = whole + ((fractional >= 128) ? 1 : 0);
        percent = (rounded > 100) ? 100 : static_cast<uint8_t>(rounded);
        return true;
    }

    static bool getVoltageMv(uint16_t& voltageMv) {
        uint16_t vcellRaw = 0;
        if (!read16(kFuelGaugeVcellRegister, vcellRaw)) return false;
        uint64_t millivolts = (static_cast<uint64_t>(vcellRaw) * 78125ULL + 500000ULL) / 1000000ULL;
        voltageMv = static_cast<uint16_t>(millivolts);
        return true;
    }

private:
    static bool read16(uint8_t reg, uint16_t& value) {
        Wire.beginTransmission(kFuelGaugeAddress);
        Wire.write(reg);
        if (Wire.endTransmission(false) != 0) return false;
        if (Wire.requestFrom(static_cast<int>(kFuelGaugeAddress), 2) != 2) return false;
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        value = (static_cast<uint16_t>(msb) << 8) | lsb;
        return true;
    }
};
