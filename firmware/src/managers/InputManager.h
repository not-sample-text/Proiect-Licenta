#pragma once
#include <Arduino.h>
#include <Keypad.h>
#include <ESP32Encoder.h>
#include "Config.h"

class InputManager {
public:
    InputManager();
    void begin();
    void update();
    
    // Returns "102" string if event occurred, else empty string
    String getEvent(); 
    bool isActivityDetected(); // For resetting sleep timer

private:
    Keypad* _matrix;
    ESP32Encoder _encoder;
    
    int _currentLayer = 1; // Default Layer
    bool _activityFlag = false;
    int64_t _lastEncPos = 0;
    
    // Internal helper to map char to coordinates
    String _formatMessage(int layer, int col, int row);
};
