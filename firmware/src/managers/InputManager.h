#pragma once
#include <Arduino.h>
#include <Keypad.h>
#include <RotaryEncoder.h>
#include <vector>
#include "../Config.h"

// Physical layout
const byte ROWS = 4;
const byte COLS = 3;
const char keys[ROWS][COLS] = {
    {'0', '1', '2'},
    {'3', '4', '5'},
    {'6', '7', '8'},
    {'9', 'A', 'B'}
};

// --- EVENT STRUCTURE ---
struct InputEvent {
    enum Type { KEY_PRESS, KEY_RELEASE, KNOB_TURN, KNOB_CLICK };
    
    Type type;      // ! FIX: Added the actual member variable
    char keyChar;   // '0'-'B' for keys
    int value;      // +1/-1 for knob, 0 for click
};

class InputManager {
private:
    Keypad* matrix;
    RotaryEncoder* encoder;
    
    int lastEncoderPos = 0;
    bool wasButtonDown = false;
    
    // Event Buffer
    std::vector<InputEvent> eventBuffer;

public:
    InputManager();
    void begin();
    
    // Scans hardware and populates buffer
    void update(); 
    
    // Returns true if event was popped into &evt
    bool getEvent(InputEvent &evt); 
};
