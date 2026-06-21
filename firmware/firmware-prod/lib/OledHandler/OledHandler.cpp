#include "OledHandler.h"
#include <U8g2lib.h>
#include "BoardSupport/pins.h"
#include "MacropadApp/MacropadApp.h"

// Full buffer mode for simplicity on ESP32-S3 (plenty of RAM)
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

uint32_t OledHandler::statusTimer = 0;
char OledHandler::statusMsg[16] = {0};

void OledHandler::begin() {
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 20, "Macropad Ready");
    u8g2.sendBuffer();
}

void OledHandler::run(bool forceRedraw) {
    bool statusActive = (statusTimer > 0 && millis() < statusTimer);
    
    if (forceRedraw || statusActive) {
        u8g2.clearBuffer();
        drawUI();
        
        if (statusActive) {
            u8g2.setDrawColor(0); // Black background for status
            u8g2.drawBox(0, 0, 128, 12);
            u8g2.setDrawColor(1);
            u8g2.drawStr(2, 10, statusMsg);
        }
        
        u8g2.sendBuffer();
        
        if (!statusActive) statusTimer = 0;
    }
}

void OledHandler::showStatus(const char* msg) {
    strncpy(statusMsg, msg, 15);
    statusTimer = millis() + 2000; // 2 seconds
}

void OledHandler::drawUI() {
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Layer info
    char layerStr[16];
    snprintf(layerStr, sizeof(layerStr), "Layer: %d", MacropadApp::getCurrentLayer());
    u8g2.drawStr(0, 10, layerStr);
    
    // Mode info
    const char* modeStr = (MacropadApp::getEncoderMode() == MacropadApp::EncoderMode::Volume) ? "Mode: VOL" : "Mode: LYR";
    u8g2.drawStr(0, 22, modeStr);
    
    // Transport info
    const char* transportStr = MacropadApp::isBleMode() ? "BLE" : "USB";
    u8g2.drawStr(90, 10, transportStr);
}
