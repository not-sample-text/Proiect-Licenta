#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "../Config.h"

class DisplayManager {
private:
    // SSD1306 128x32 using Hardware I2C
    U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C* u8g2;

public:
    DisplayManager();
    void begin();
    
    // We will call this whenever state changes
    void drawScreen(String layerName, bool isUsbConnected, int batteryLevel);
    
    // Helper for boot messages
    void showMessage(String msg);
};
