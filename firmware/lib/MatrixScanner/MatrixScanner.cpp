#include "MatrixScanner.h"
#include "BoardSupport/pins.h"
#include "Core/EventQueue.h"

bool MatrixScanner::currentState[4][3] = {false};
uint32_t MatrixScanner::lastDebounceTime[4][3] = {0};

void MatrixScanner::begin() {
    // Rows are inputs with internal pull-up
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        pinMode(ROWS_PINS[r], INPUT_PULLUP);
    }

    // Columns are outputs, default to HIGH (inactive)
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        pinMode(COLS_PINS[c], OUTPUT);
        digitalWrite(COLS_PINS[c], HIGH);
    }
}

void MatrixScanner::scan() {
    uint32_t now = millis();

    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        // Activate column (pull to LOW)
        digitalWrite(COLS_PINS[c], LOW);
        
        // Small delay not needed if pins are slow, but ESP32 is fast. 
        // However, instructions say "Fără delay()", and usually a few NOPs or just the overhead of digitalWrite is enough.
        
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            // Read active-low state (LOW = pressed)
            bool rawState = (digitalRead(ROWS_PINS[r]) == LOW);

            // If raw state differs from stable state, reset debounce timer
            if (rawState != currentState[r][c]) {
                if (now - lastDebounceTime[r][c] > DEBOUNCE_DELAY_MS) {
                    // State has been stable for long enough, accept it
                    currentState[r][c] = rawState;
                    lastDebounceTime[r][c] = now;

                    // Enqueue event
                    EventType type = rawState ? EventType::KeyPress : EventType::KeyRelease;
                    EventQueue::enqueue({type, r, c, now});
                }
            } else {
                // State matches current stable state, update timer to "now" 
                // to ensure we need a *continuous* 5ms of difference to trigger.
                lastDebounceTime[r][c] = now;
            }
        }

        // Deactivate column (return to HIGH)
        digitalWrite(COLS_PINS[c], HIGH);
    }
}
