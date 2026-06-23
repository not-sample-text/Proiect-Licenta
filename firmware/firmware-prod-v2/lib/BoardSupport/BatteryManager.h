#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

class BatteryManager {
public:
    static bool getPercent(uint8_t& percent) {
        uint16_t voltageMv = 0;
        if (!getVoltageMv(voltageMv)) return false;

        // Custom piecewise discharge curve for 650mAh 3.7V LiPo
        if (voltageMv >= 4100) {
            percent = 100;
        } else if (voltageMv >= 3850) {
            percent = map(voltageMv, 3850, 4100, 50, 100);
        } else if (voltageMv >= 3650) {
            percent = map(voltageMv, 3650, 3850, 10, 50);
        } else if (voltageMv >= 3300) {
            percent = map(voltageMv, 3300, 3650, 0, 10);
        } else {
            percent = 0;
        }
        
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
