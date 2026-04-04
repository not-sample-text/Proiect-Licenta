/**
 * @file rgb.cpp
 * @brief RGB underglow implementation using FastLED.
 */

#include "Rgb.h"
#include "pins.h"
#include "Config.h"
#include "debug.h"
#include <FastLED.h>

// ── Hardware Configuration ──────────────────────────────────────
#define RGB_MAX_BRIGHTNESS  128  // Cap brightness to protect power draw

// ── LED Array ───────────────────────────────────────────────────
CRGB g_leds[RGB_LED_COUNT];

// ── State Variables ─────────────────────────────────────────────
static bool     g_rgb_initialized = false;
static RGBMode  g_rgb_mode = RGB_MODE_SOLID;
static uint8_t  g_rgb_brightness = 128;
static uint8_t  g_rgb_speed = 128;
static uint8_t  g_rgb_r = 255;
static uint8_t  g_rgb_g = 0;
static uint8_t  g_rgb_b = 255;  // Default: magenta

// Animation state
static uint32_t g_last_update = 0;
static uint16_t g_animation_frame = 0;
static bool     g_rgb_powered_off = false;

// ── Initialization ──────────────────────────────────────────────
bool rgb_init() {
    DBG_INFO("RGB", "Initializing RGB underglow...");
    
    // Initialize FastLED
    FastLED.addLeds<SK6812, PIN_RGB_LEDS, GRB>(g_leds, RGB_LED_COUNT);
    FastLED.setBrightness(RGB_MAX_BRIGHTNESS);
    FastLED.clear(true);
    
    // Load settings from config
    const RGBConfig& cfg = config_get_rgb();
    g_rgb_brightness = (cfg.brightness < RGB_MAX_BRIGHTNESS) ? cfg.brightness : RGB_MAX_BRIGHTNESS;
    g_rgb_mode = (RGBMode)cfg.mode;
    g_rgb_speed = cfg.speed;
    
    // Extract RGB from color
    g_rgb_r = (cfg.color >> 16) & 0xFF;
    g_rgb_g = (cfg.color >> 8) & 0xFF;
    g_rgb_b = cfg.color & 0xFF;
    
    FastLED.setBrightness(g_rgb_brightness);
    
    g_rgb_initialized = true;
    DBG_INFO("RGB", "RGB initialized: mode=%d, brightness=%d, color=#%06X", 
             g_rgb_mode, g_rgb_brightness, cfg.color);
    
    return true;
}

// ── Color Utilities ─────────────────────────────────────────────
static CRGB wheel(uint8_t pos) {
    // Input 0-255 to get rainbow color
    pos = 255 - pos;
    if (pos < 85) {
        return CRGB(255 - pos * 3, 0, pos * 3);
    }
    if (pos < 170) {
        pos -= 85;
        return CRGB(0, pos * 3, 255 - pos * 3);
    }
    pos -= 170;
    return CRGB(pos * 3, 255 - pos * 3, 0);
}

// ── Effect Implementations ──────────────────────────────────────
static void effect_solid() {
    for (int i = 0; i < RGB_LED_COUNT; i++) {
        g_leds[i] = CRGB(g_rgb_r, g_rgb_g, g_rgb_b);
    }
    FastLED.show();
}

static void effect_breathing() {
    // Breathing effect: sine wave brightness modulation
    float breathe = (sin(g_animation_frame * 0.02) + 1.0) / 2.0;  // 0.0 to 1.0
    uint8_t r = g_rgb_r * breathe;
    uint8_t g = g_rgb_g * breathe;
    uint8_t b = g_rgb_b * breathe;
    
    for (int i = 0; i < RGB_LED_COUNT; i++) {
        g_leds[i] = CRGB(r, g, b);
    }
    FastLED.show();
    
    g_animation_frame++;
}

static void effect_rainbow() {
    // Static rainbow across all LEDs
    for (int i = 0; i < RGB_LED_COUNT; i++) {
        uint8_t hue = (i * 256 / RGB_LED_COUNT) & 0xFF;
        g_leds[i] = wheel(hue);
    }
    FastLED.show();
}

static void effect_cycle() {
    // Rotating rainbow
    for (int i = 0; i < RGB_LED_COUNT; i++) {
        uint8_t hue = ((i * 256 / RGB_LED_COUNT) + g_animation_frame) & 0xFF;
        g_leds[i] = wheel(hue);
    }
    FastLED.show();
    
    g_animation_frame++;
    if (g_animation_frame >= 256) {
        g_animation_frame = 0;
    }
}

// ── Updates ─────────────────────────────────────────────────────
void rgb_update() {
    if (!g_rgb_initialized || g_rgb_powered_off) {
        return;
    }
    
    if (g_rgb_mode == RGB_MODE_OFF) {
        FastLED.clear(true);
        return;
    }
    
    // Calculate update interval based on speed
    // speed 0 = 100ms, speed 255 = 10ms
    uint32_t interval = 100 - (g_rgb_speed * 90 / 255);
    
    if (millis() - g_last_update < interval) {
        return;
    }
    g_last_update = millis();
    
    // Execute current effect
    switch (g_rgb_mode) {
        case RGB_MODE_SOLID:
            effect_solid();
            break;
        case RGB_MODE_BREATHING:
            effect_breathing();
            break;
        case RGB_MODE_RAINBOW:
            effect_rainbow();
            break;
        case RGB_MODE_CYCLE:
            effect_cycle();
            break;
        default:
            break;
    }
}

// ── Settings ────────────────────────────────────────────────────
void rgb_set_mode(RGBMode mode) {
    if (mode >= RGB_MODE_COUNT) {
        mode = RGB_MODE_SOLID;
    }
    
    if (g_rgb_mode != mode) {
        g_rgb_mode = mode;
        g_animation_frame = 0;
        DBG_INFO("RGB", "RGB mode: %d", mode);
    }
}

void rgb_set_color(uint8_t r, uint8_t g, uint8_t b) {
    g_rgb_r = r;
    g_rgb_g = g;
    g_rgb_b = b;
    DBG_INFO("RGB", "RGB color: #%02X%02X%02X", r, g, b);
}

void rgb_set_brightness(uint8_t brightness) {
    g_rgb_brightness = (brightness < RGB_MAX_BRIGHTNESS) ? brightness : RGB_MAX_BRIGHTNESS;
    FastLED.setBrightness(g_rgb_brightness);
    DBG_INFO("RGB", "RGB brightness: %d", g_rgb_brightness);
}

void rgb_set_speed(uint8_t speed) {
    g_rgb_speed = speed;
    DBG_INFO("RGB", "RGB speed: %d", speed);
}

// ── Power Management ────────────────────────────────────────────
void rgb_off() {
    if (!g_rgb_initialized || g_rgb_powered_off) {
        return;
    }
    
    FastLED.clear(true);
    g_rgb_powered_off = true;
    DBG_VERBOSE("RGB", "RGB powered off");
}

void rgb_restore() {
    if (!g_rgb_initialized || !g_rgb_powered_off) {
        return;
    }
    
    g_rgb_powered_off = false;
    g_animation_frame = 0;
    DBG_VERBOSE("RGB", "RGB restored");
}
