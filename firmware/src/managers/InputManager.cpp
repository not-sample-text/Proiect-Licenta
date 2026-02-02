#include "InputManager.h"

// Define the Keymap just to get indices
char keys[MATRIX_ROWS][MATRIX_COLS] = {
  {'0','1','2'}, // Row 0
  {'3','4','5'}, // Row 1
  {'6','7','8'}  // Row 2
};

InputManager::InputManager() {
    _matrix = new Keypad(makeKeymap(keys), (byte*)ROW_PINS, (byte*)COL_PINS, MATRIX_ROWS, MATRIX_COLS);
}

void InputManager::begin() {
    ESP32Encoder::useInternalWeakPullResistors = UP;
    _encoder.attachHalfQuad(PIN_ENC_A, PIN_ENC_B);
    pinMode(PIN_ENC_BTN, INPUT_PULLUP);
}

String InputManager::getEvent() {
    String eventMsg = "";
    
    // 1. Check Matrix
    // getKey() returns the char we defined in 'keys' array
    char keyChar = _matrix->getKey(); 
    
    if (keyChar) {
        _activityFlag = true;
        
        // Reverse engineer Col/Row from the char
        // (Since '0' is 48 in ASCII, we can do math)
        int keyIndex = keyChar - '0'; 
        int row = keyIndex / MATRIX_COLS;
        int col = keyIndex % MATRIX_COLS;
        
        eventMsg = _formatMessage(_currentLayer, col, row);
    }

    // 2. Check Encoder Rotation
    int64_t newPos = _encoder.getCount();
    if (newPos != _lastEncPos) {
        _activityFlag = true;
        // Example: Encoder sends special codes? 
        // Or changes layer? Let's say CW = Layer Up, CCW = Layer Down
        if (newPos > _lastEncPos) _currentLayer++;
        else _currentLayer--;
        
        // Clamp Layer 0-9
        if (_currentLayer > 9) _currentLayer = 0;
        if (_currentLayer < 0) _currentLayer = 9;
        
        _lastEncPos = newPos;
        // Optional: Send a "Layer Changed" event or just stay silent
        Serial.println("Layer: " + String(_currentLayer));
    }

    // 3. Check Encoder Button
    if (digitalRead(PIN_ENC_BTN) == LOW) {
        // Simple debounce needed here in real life
        delay(50); 
        if (digitalRead(PIN_ENC_BTN) == LOW) {
            _activityFlag = true;
            // Maybe Button sends "100" (Layer 1, Col 0, Row 0 virtual)?
            // For now, let's treat it as a layer reset
             _currentLayer = 1;
             while(digitalRead(PIN_ENC_BTN) == LOW); // Wait release
        }
    }

    return eventMsg;
}

bool InputManager::isActivityDetected() {
    bool state = _activityFlag;
    _activityFlag = false; // Reset on read
    return state;
}

String InputManager::_formatMessage(int layer, int col, int row) {
    // Protocol: LCR (e.g. "102")
    return String(layer) + String(col) + String(row);
}
