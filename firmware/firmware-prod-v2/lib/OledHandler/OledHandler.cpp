#include "OledHandler.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// External linkages to cross-module headers
#include "MacropadApp.h"
#include "BoardSupport.h"

// Instantiate the static class member for the SSD1306 display instance
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 OledHandler::display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Link against loose global properties instantiated in MacropadApp.cpp
extern uint8_t currentEncoderMode; 
extern uint32_t bleSecurePasskeyCached;
extern bool displayPasskeyActive;

// Custom monochrome bitmaps for status indicators
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

// Helper to resolve long workspace layer names dynamically
const char* getLayerLongName(uint8_t layer) {
    switch (layer) {
        case 0:  return "FN KEYS";
        case 1:  return "SHORTCUTS";
        case 2:  return "COMMANDS";
        case 3:  return "APPS";
        default: return "UNKNOWN";
    }
}

// Helper to resolve the exact label of the active key nickname mapping context
const char* getLastKeyNickname() {
    // FIX: Using your public getter getLastKey() instead of direct private access
    char activeKey = MacropadApp::getLastKey();

    if (activeKey == '-') return "IDLE";
    if (activeKey == 'S') return "SERIAL DISPATCH";
    
    if (activeKey == 'C') return "COPY";
    if (activeKey == 'V') return "PASTE";
    if (activeKey == 'A') return "SELECT ALL";
    if (activeKey == 'Z') return "UNDO";
    if (activeKey == 'X') return "CUT";
    if (activeKey == 'W') return "GFX RESET";
    if (activeKey == 'T') return "TAB";
    if (activeKey == 'S') return "SPACE";
    if (activeKey == 'E') return "ENTER";
    if (activeKey == 'B') return "BACKSPACE";
    
    static char fallback[16];
    snprintf(fallback, 16, "KEY F%d", activeKey);
    return fallback;
}

// Helper to resolve the active encoder functionality tracking string context
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

    // ─────────────────────────────────────────────────────────────────
    // CONDITION: BLE PASSKEY SECURITY AUTHENTICATION SCREEN ACTIVE
    // ─────────────────────────────────────────────────────────────────
    // FIX: Using your public getter isBleMode() instead of direct private access
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

    // ─────────────────────────────────────────────────────────────────
    // STANDARD: 3-ROW USER INTERFACE STRUCTURE EXECUTION
    // ─────────────────────────────────────────────────────────────────
    
    // ── ROW 1: THE STATUS BAR ──
    display.setFont(nullptr);
    display.setTextSize(1);
    display.setCursor(0, 2);
    // FIX: Using your public getter getCurrentLayer() instead of direct private access
    display.print(getLayerLongName(MacropadApp::getCurrentLayer())); 

    int currentX = 120;

    // A. Render Battery Percentage
    uint8_t currentBatPercent = 57; 
    char batStr[6];
    snprintf(batStr, 6, "%d%%", currentBatPercent);
    currentX -= (strlen(batStr) * 6);
    display.setCursor(currentX, 2);
    display.print(batStr);

    // B. Render Dynamic Charging Status Bolt
    if (BoardSupport::isUsbConnected()) {
        currentX -= 7;
        display.drawBitmap(currentX, 1, lightning_bolt, 5, 9, SSD1306_WHITE);
    }

    // C. Render Wireless Bluetooth Presence Indicator Icon
    // FIX: Using your public getter isBleMode() instead of direct private access
    if (MacropadApp::isBleMode()) {
        currentX -= 8;
        display.drawBitmap(currentX, 1, bluetooth_icon, 5, 9, SSD1306_WHITE);
    }

    display.drawFastHLine(0, 13, 128, SSD1306_WHITE);

    // ── ROW 2: THE DATA REGISTRY CENTERPIECE ──
    display.setTextSize(2); 
    const char* nickname = getLastKeyNickname();
    
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(nickname, 0, 30, &x1, &y1, &w, &h);
    int centerTextX = (128 - w) / 2;
    if (centerTextX < 0) centerTextX = 0; 
    
    display.setCursor(centerTextX, 26);
    display.print(nickname);

    display.drawFastHLine(0, 49, 128, SSD1306_WHITE);

    // ── ROW 3: THE ENCODER CONTROLS FOOTER ──
    display.setTextSize(1);
    display.setCursor(0, 54);
    display.print(getEncoderModeString()); 

    display.display();
}

void OledHandler::nextScreen() {}
void OledHandler::previousScreen() {}

void OledHandler::showSleepAnimation() {
    for (int16_t i = 0; i < 64; i += 4) {
        display.drawFastHLine(0, i, 128, SSD1306_BLACK);
        display.drawFastHLine(0, 63 - i, 128, SSD1306_BLACK);
        display.display();
        delay(15);
    }
    display.clearDisplay();
    display.display();
}
