#pragma once

#include <Arduino.h>

/**
 * @brief Main application orchestrator.
 */
class MacropadApp {
public:
    enum class EncoderMode : uint8_t { Volume, Layer };

    static void begin();
    static void run();

    static uint8_t getCurrentLayer() { return currentLayer; }
    static EncoderMode getEncoderMode() { return encoderMode; }
    static bool isBleMode() { return g_ble_mode; }

private:
    static void processEvents();
    
    static uint8_t currentLayer;
    static EncoderMode encoderMode;
    static bool needsDisplayUpdate;
    static bool g_ble_mode;
};
