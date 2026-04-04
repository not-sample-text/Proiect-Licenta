/**
 * @file oled.cpp
 * @brief OLED display implementation using U8g2 library.
 */

#include "Oled.h"
#include "pins.h"
#include "debug.h"
#include "Power.h"
#include <U8g2lib.h>
#include <Wire.h>

// ── Display Object ──────────────────────────────────────────────
// SSD1306 128x32 I2C display
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ── State Variables ─────────────────────────────────────────────
static bool     g_oled_initialized = false;
static bool     g_oled_sleeping = false;
static String   g_current_layer = "FN Keys";
static String   g_connection_mode = "USB";
static String   g_last_key = "";
static String   g_encoder_mode = "VOLUME";
static String   g_status_message = "";
static uint32_t g_status_timeout = 0;
static bool     g_needs_redraw = true;

// ── Initialization ──────────────────────────────────────────────
bool oled_init() {
    DBG_INFO("OLED", "Initializing OLED display...");
    
    // Initialize I2C with custom pins
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    
    // Initialize U8g2
    if (!u8g2.begin()) {
        DBG_ERROR("OLED", "OLED initialization failed");
        return false;
    }
    
    // Set font and contrast
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setContrast(255);  // Max brightness initially
    
    // Show boot splash
    u8g2.clearBuffer();
    u8g2.drawStr(10, 15, "PROS3 Macropad");
    u8g2.drawStr(25, 28, "Initializing...");
    u8g2.sendBuffer();
    
    g_oled_initialized = true;
    DBG_INFO("OLED", "OLED initialized OK");
    
    // Clear splash after a moment
    delay(1000);
    g_needs_redraw = true;
    
    return true;
}

// ── Layout Helper ───────────────────────────────────────────────
static void draw_battery_icon(int x, int y, uint8_t percent, bool charging) {
    // Draw battery outline (10x6 pixels)
    u8g2.drawFrame(x, y, 10, 6);
    u8g2.drawBox(x + 10, y + 2, 2, 2);  // Battery terminal
    
    // Fill battery based on percentage
    int fill_width = (percent * 8) / 100;  // 8 pixels max fill
    if (fill_width > 0) {
        u8g2.drawBox(x + 1, y + 1, fill_width, 4);
    }
    
    // Draw lightning symbol if charging
    if (charging) {
        u8g2.setDrawColor(2);  // XOR mode
        u8g2.drawLine(x + 6, y + 1, x + 4, y + 3);
        u8g2.drawLine(x + 4, y + 3, x + 6, y + 5);
        u8g2.setDrawColor(1);  // Back to normal
    }
}

static void draw_layout() {
    u8g2.clearBuffer();
    
    // Status message takes priority if active
    if (g_status_message.length() > 0) {
        if (g_status_timeout > 0 && millis() > g_status_timeout) {
            g_status_message = "";
            g_status_timeout = 0;
            g_needs_redraw = true;
            return;
        }
        
        // Center the status message
        u8g2.setFont(u8g2_font_6x10_tf);
        int width = u8g2.getStrWidth(g_status_message.c_str());
        int x = (128 - width) / 2;
        u8g2.drawStr(x, 16, g_status_message.c_str());
        u8g2.sendBuffer();
        return;
    }
    
    // ── Top Row ─────────────────────────────────────────────────
    // Left: Layer name (small font)
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(0, 8, g_current_layer.c_str());
    
    // Right side: Icons and battery
    int right_x = 128;  // Start from right edge
    
    // Battery percentage text
    uint8_t battery_percent = power_get_battery_percent();
    char battery_text[8];
    snprintf(battery_text, sizeof(battery_text), "%d%%", battery_percent);
    int text_width = u8g2.getStrWidth(battery_text);
    right_x -= text_width;
    u8g2.drawStr(right_x, 8, battery_text);
    
    // Battery icon
    right_x -= 16;  // Space for icon
    draw_battery_icon(right_x, 2, battery_percent, power_is_charging());
    
    // Bluetooth icon (if BLE mode)
    if (g_connection_mode == "BLE") {
        right_x -= 10;  // Space for BT icon
        // Draw Bluetooth icon (simplified ᛒ rune)
        u8g2.drawLine(right_x + 3, 2, right_x + 3, 9);    // Vertical line
        u8g2.drawLine(right_x + 3, 2, right_x + 6, 5);    // Top right
        u8g2.drawLine(right_x + 3, 9, right_x + 6, 6);    // Bottom right
        u8g2.drawLine(right_x + 3, 5, right_x, 2);        // Top left cross
        u8g2.drawLine(right_x + 3, 6, right_x, 9);        // Bottom left cross
    }
    
    // ── Bottom Row ──────────────────────────────────────────────
    // Encoder mode (left)
    u8g2.setFont(u8g2_font_5x8_tf);
    char mode_text[16];
    snprintf(mode_text, sizeof(mode_text), "<%s>", g_encoder_mode.c_str());
    u8g2.drawStr(0, 28, mode_text);
    
    // Last pressed key (center/right) - larger font if room
    if (g_last_key.length() > 0) {
        u8g2.setFont(u8g2_font_6x10_tf);
        int key_width = u8g2.getStrWidth(g_last_key.c_str());
        
        // Truncate if too long
        String display_key = g_last_key;
        if (key_width > 80) {  // Leave room for encoder mode
            // Shorten to fit
            while (key_width > 80 && display_key.length() > 3) {
                display_key = display_key.substring(0, display_key.length() - 1);
                display_key += ".";
                key_width = u8g2.getStrWidth(display_key.c_str());
            }
        }
        
        // Right-align the key label
        int key_x = 128 - key_width;
        u8g2.drawStr(key_x, 28, display_key.c_str());
    }
    
    u8g2.sendBuffer();
}

// ── Display Updates ─────────────────────────────────────────────
void oled_update() {
    if (!g_oled_initialized || g_oled_sleeping) {
        return;
    }
    
    if (g_needs_redraw) {
        draw_layout();
        g_needs_redraw = false;
    }
}

void oled_refresh() {
    g_needs_redraw = true;
    oled_update();
}

// ── Content Updates ─────────────────────────────────────────────
void oled_set_layer(const char* layer_name) {
    if (g_current_layer != layer_name) {
        g_current_layer = layer_name;
        g_needs_redraw = true;
    }
}

void oled_set_connection_mode(const char* mode) {
    if (g_connection_mode != mode) {
        g_connection_mode = mode;
        g_needs_redraw = true;
    }
}

void oled_set_last_key(const char* label) {
    if (label && g_last_key != label) {
        g_last_key = label;
        g_needs_redraw = true;
    }
}

void oled_set_encoder_mode(const char* mode) {
    if (g_encoder_mode != mode) {
        g_encoder_mode = mode;
        g_needs_redraw = true;
    }
}

void oled_show_status(const char* message, uint32_t timeout_ms) {
    g_status_message = message;
    if (timeout_ms > 0) {
        g_status_timeout = millis() + timeout_ms;
    } else {
        g_status_timeout = 0;
    }
    g_needs_redraw = true;
    oled_update();  // Immediate update for status messages
}

void oled_clear_status() {
    if (g_status_message.length() > 0) {
        g_status_message = "";
        g_status_timeout = 0;
        g_needs_redraw = true;
    }
}

// ── Power Management ────────────────────────────────────────────
void oled_sleep() {
    if (!g_oled_initialized || g_oled_sleeping) {
        return;
    }
    
    u8g2.setPowerSave(1);  // Turn off display
    g_oled_sleeping = true;
    DBG_VERBOSE("OLED", "OLED sleeping");
}

void oled_wake() {
    if (!g_oled_initialized || !g_oled_sleeping) {
        return;
    }
    
    u8g2.setPowerSave(0);  // Turn on display
    g_oled_sleeping = false;
    g_needs_redraw = true;
    DBG_VERBOSE("OLED", "OLED wake");
}

void oled_dim() {
    if (!g_oled_initialized || g_oled_sleeping) {
        return;
    }
    
    u8g2.setContrast(64);  // Low brightness
    DBG_VERBOSE("OLED", "OLED dimmed");
}

void oled_bright() {
    if (!g_oled_initialized || g_oled_sleeping) {
        return;
    }
    
    u8g2.setContrast(255);  // Full brightness
    DBG_VERBOSE("OLED", "OLED bright");
}
