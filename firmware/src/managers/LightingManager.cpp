#include "LightingManager.h"

LightingManager::LightingManager() {
    currentMode = MODE_RAINBOW; // Default to rainbow to test hardware
    solidColor = CRGB::Red;
    brightness = 128;
    hue = 0;
}

void LightingManager::begin() {
    // Initialize FastLED for SK6812 (NeoPixel compatible)
    // Pin 7 is defined in Config.h
    FastLED.addLeds<SK6812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(brightness);
    
    // Quick startup test: Flash Red
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
    delay(200);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    
    Serial.println("[Lighting] Initialized");
}

void LightingManager::update() {
    // Non-blocking animation updates
    // In a real app, check millis() here to control speed.
    // For now, we run every loop cycle (fast).
    
    if (currentMode == MODE_OFF) {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
    }
    else if (currentMode == MODE_SOLID) {
        fill_solid(leds, NUM_LEDS, solidColor);
    }
    else if (currentMode == MODE_RAINBOW) {
        fill_rainbow(leds, NUM_LEDS, hue, 7); // 7 is the delta hue between leds
        hue++; // Cycle color
    }
    
    FastLED.show();
    // Small delay to keep rainbow speed reasonable if this is called fast
    delay(10); 
}

void LightingManager::setMode(LightingMode mode) {
    currentMode = mode;
}

void LightingManager::setColor(CRGB color) {
    solidColor = color;
}

void LightingManager::setBrightness(uint8_t b) {
    brightness = b;
    FastLED.setBrightness(brightness);
}
