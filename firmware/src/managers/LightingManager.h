#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "../Config.h"

enum LightingMode {
    MODE_SOLID,
    MODE_RAINBOW,
    MODE_OFF
};

class LightingManager {
private:
    CRGB leds[NUM_LEDS];
    LightingMode currentMode;
    CRGB solidColor;
    uint8_t brightness;
    uint8_t hue; // For rainbow effect

public:
    LightingManager();
    void begin();
    void update(); // Called every loop
    
    // Control methods
    void setMode(LightingMode mode);
    void setColor(CRGB color);
    void setBrightness(uint8_t b);
};
