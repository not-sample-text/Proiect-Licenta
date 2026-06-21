#include "RgbHandler.h"
#define FASTLED_INTERNAL // Disable version banner
#include <FastLED.h>
#include "BoardSupport/pins.h"

CRGB leds[RGB_COUNT];
uint32_t RgbHandler::lastUpdate = 0;
uint8_t RgbHandler::hue = 0;

void RgbHandler::begin() {
    FastLED.addLeds<SK6812, PIN_RGB_DATA, GRB>(leds, RGB_COUNT);
    FastLED.setBrightness(32);
}

void RgbHandler::run() {
    uint32_t now = millis();
    if (now - lastUpdate > 30) { // 33 FPS
        lastUpdate = now;
        
        fill_rainbow(leds, RGB_COUNT, hue++, 20);
        FastLED.show();
    }
}
