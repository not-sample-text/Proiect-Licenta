#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

enum class Screen : uint8_t {
    kMatrix = 0,
    kBattery = 1,
    kInputs = 2,
    kDebug = 3,
    kEncoderDebug = 4
};

class OledHandler {
public:
    static void begin();
    static void run();
    static void update();
    
    // Navigation hooks
    static void nextScreen();
    static void previousScreen();
    static Screen getCurrentScreen();

    // Hardware animation transitions
    static void showBootAnimation();
    static void showSleepAnimation();
    static void clear();

private:
    static void drawTextCentered(int16_t y, const char* text);
    static void showMatrixScreen();
    static void showBatteryScreen();
    static void showInputsScreen();
    static void showDebugScreen();
    static void showEncoderDebugScreen();

    static Adafruit_SSD1306 display;
    static Screen currentScreen;
};
