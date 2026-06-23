#include "OledHandler.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "MacropadApp.h"
#include "BoardSupport.h"
#include "BatteryManager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 OledHandler::display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

extern uint8_t currentEncoderMode; 
extern uint32_t bleSecurePasskeyCached;
extern bool displayPasskeyActive;

static const unsigned char PROGMEM bluetooth_icon[] = {
    0x10, 0x30, 0x54, 0x38, 0x10, 0x38, 0x54, 0x30, 0x10
}; 

static const unsigned char PROGMEM lightning_bolt[] = {
    0x20, 0x60, 0x40, 0xC0, 0xF8, 0x10, 0x30, 0x20, 0x40
};

void OledHandler::begin() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        return;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
}

const char* getLayerLongName(uint8_t layer) {
    switch (layer) {
        case 0:  return "FN KEYS";
        case 1:  return "SHORTCUTS";
        case 2:  return "COMMANDS";
        case 3:  return "APPS";
        default: return "UNKNOWN";
    }
}

const char* getEncoderModeString() {
    switch (currentEncoderMode) {
        case 0:  return "NAVIGATION";
        case 1:  return "VOLUME";
        case 2:  return "LAYER SELECT";
        default: return "UNKNOWN";
    }
}

void OledHandler::update() {
    display.clearDisplay();
    display.setTextWrap(false);

    if (MacropadApp::isBleMode() && displayPasskeyActive) {
        display.setFont(nullptr);
        display.setTextSize(1);
        display.setCursor(16, 4);
        display.print("BLE PAIRING PASSKEY:");
        
        display.setTextSize(3);
        char passkeyStr[8];
        snprintf(passkeyStr, 8, "%06u", bleSecurePasskeyCached);
        
        display.setCursor(10, 24);
        display.print(passkeyStr);
        display.display();
        return;
    }

    display.setFont(nullptr);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(getLayerLongName(MacropadApp::getCurrentLayer())); 

    int currentX = 120;

    // Grab the real battery percentage instead of the hardcoded 57%
    uint8_t currentBatPercent = 0; 
    BatteryManager::getPercent(currentBatPercent);
    
    char batStr[6];
    snprintf(batStr, 6, "%d%%", currentBatPercent);
    currentX -= (strlen(batStr) * 6);
    display.setCursor(currentX, 0);
    display.print(batStr);

    if (BoardSupport::isUsbConnected()) {
        currentX -= 7;
        display.drawBitmap(currentX, 0, lightning_bolt, 5, 9, SSD1306_WHITE);
    }

    if (MacropadApp::isBleMode()) {
        currentX -= 8;
        display.drawBitmap(currentX, 0, bluetooth_icon, 5, 9, SSD1306_WHITE);
    }

    display.setTextSize(1); 
    const char* nickname = MacropadApp::getLastKeyLabel();
    
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(nickname, 0, 12, &x1, &y1, &w, &h);
    int centerTextX = (128 - w) / 2;
    if (centerTextX < 0) centerTextX = 0; 
    
    display.setCursor(centerTextX, 12);
    display.print(nickname);

    display.setTextSize(1);
    display.setCursor(0, 24);
    display.print(getEncoderModeString()); 

    display.display();
}

void OledHandler::nextScreen() {}
void OledHandler::previousScreen() {}

void OledHandler::clear() {
    display.clearDisplay();
    display.display();
}

void OledHandler::showBootAnimation() {
    display.clearDisplay();
    for (int16_t i = 0; i < 64; i += 8) {
        display.drawFastHLine(64 - i, 16, i * 2, SSD1306_WHITE);
        display.display();
        delay(15);
    }
    delay(150);
}

void OledHandler::showSleepAnimation() {
    for (int16_t i = 0; i < 16; i += 2) {
        display.drawFastHLine(0, i, 128, SSD1306_WHITE);
        display.drawFastHLine(0, 31 - i, 128, SSD1306_WHITE);
        display.display();
        delay(20);
    }
    display.clearDisplay();
    display.display();
}
