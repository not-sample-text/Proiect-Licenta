#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

class OledHandler {
public:
    static void begin();
    static void update();
    
    // Hardware animation transitions
    static void showBootAnimation();
    static void showSleepAnimation();
    static void clear();

private:
    static Adafruit_SSD1306 display;
};
