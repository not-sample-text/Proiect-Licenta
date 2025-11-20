#include "InputManager.h"

InputManager::InputManager() {
    matrix = new Keypad(makeKeymap(keys), (byte*)ROW_PINS, (byte*)COL_PINS, ROWS, COLS);
    encoder = new RotaryEncoder(ENC_PIN_A, ENC_PIN_B, RotaryEncoder::LatchMode::TWO03);
}

void InputManager::begin() {
    pinMode(ENC_BUTTON, INPUT_PULLUP);
    pinMode(BT_SELECT_PIN, INPUT_PULLUP);
    Serial.println("[Input] Initialized");
}

void InputManager::update() {
    // 1. SCAN MATRIX
    if (matrix->getKeys()) {
        for (int i = 0; i < LIST_MAX; i++) {
            if (matrix->key[i].stateChanged) {
                InputEvent evt;
                evt.keyChar = matrix->key[i].kchar;
                
                if (matrix->key[i].kstate == PRESSED) {
                    evt.type = InputEvent::KEY_PRESS;
                    eventBuffer.push_back(evt);
                } 
                else if (matrix->key[i].kstate == RELEASED) {
                    evt.type = InputEvent::KEY_RELEASE;
                    eventBuffer.push_back(evt);
                }
            }
        }
    }

    // 2. SCAN KNOB
    encoder->tick();
    int newPos = encoder->getPosition();
    if (lastEncoderPos != newPos) {
        InputEvent evt;
        evt.type = InputEvent::KNOB_TURN;
        evt.value = (int)(encoder->getDirection()); // +1 or -1
        eventBuffer.push_back(evt);
        lastEncoderPos = newPos;
    }

    // 3. SCAN KNOB BUTTON
    bool isButtonDown = (digitalRead(ENC_BUTTON) == LOW);
    if (isButtonDown && !wasButtonDown) {
        InputEvent evt;
        evt.type = InputEvent::KNOB_CLICK;
        eventBuffer.push_back(evt);
    }
    wasButtonDown = isButtonDown;
}

bool InputManager::getEvent(InputEvent &evt) {
    if (eventBuffer.empty()) return false;
    
    // Pop the oldest event (FIFO)
    evt = eventBuffer.front();
    eventBuffer.erase(eventBuffer.begin());
    return true;
}
