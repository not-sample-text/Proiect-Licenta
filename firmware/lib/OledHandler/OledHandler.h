#pragma once

#include <Arduino.h>

/**
 * @brief Handles the SSD1306 OLED display using U8g2.
 */
class OledHandler {
public:
    static void begin();
    
    /**
     * @brief Redraws the display if dirty or status changed.
     * @param forceRedraw True if application state changed (e.g. key press).
     */
    static void run(bool forceRedraw);

    /**
     * @brief Shows a temporary status message.
     */
    static void showStatus(const char* msg);

private:
    static void drawUI();
    
    static uint32_t statusTimer;
    static char statusMsg[16];
};
