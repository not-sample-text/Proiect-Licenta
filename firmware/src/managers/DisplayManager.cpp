#include "DisplayManager.h"

DisplayManager::DisplayManager() {
    // Rotation R0 = Normal, R2 = 180 degrees (if screen is upside down)
    u8g2 = new U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
}

void DisplayManager::begin() {
    // Configure I2C Pins for ESP32-S3
    Wire.setPins(OLED_SDA, OLED_SCL);
    
    u8g2->begin();
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_ncenB08_tr); // Choose a nice small font
    u8g2->drawStr(0, 10, "Booting...");
    u8g2->sendBuffer();
    
    Serial.println("[Display] Initialized");
}

void DisplayManager::showMessage(String msg) {
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_ncenB08_tr);
    u8g2->setCursor(0, 20);
    u8g2->print(msg);
    u8g2->sendBuffer();
}

void DisplayManager::drawScreen(String layerName, bool isUsbConnected, int batteryLevel) {
    u8g2->clearBuffer();
    
    // 1. Draw Status Bar (Top Line)
    u8g2->setFont(u8g2_font_profont10_mr); // Tiny font
    
    // Connection Icon
    if (isUsbConnected) {
        u8g2->drawStr(0, 8, "USB");
    } else {
        u8g2->drawStr(0, 8, "BLE");
    }
    
    // Battery % (Right Aligned)
    String batt = String(batteryLevel) + "%";
    int w = u8g2->getStrWidth(batt.c_str());
    u8g2->setCursor(128 - w, 8);
    u8g2->print(batt);
    
    // Divider Line
    u8g2->drawLine(0, 10, 128, 10);
    
    // 2. Draw Main Layer Name (Centered)
    u8g2->setFont(u8g2_font_helvB12_tr); // Big Bold font
    int nameW = u8g2->getStrWidth(layerName.c_str());
    u8g2->setCursor((128 - nameW) / 2, 28); // Center horizontally
    u8g2->print(layerName);
    
    u8g2->sendBuffer();
}
