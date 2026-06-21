#include "EncoderHandler.h"
#include "BoardSupport/pins.h"
#include "Core/EventQueue.h"

volatile int8_t EncoderHandler::encoderDelta = 0;
uint8_t EncoderHandler::lastState = 0;
bool EncoderHandler::lastButtonState = true; // Pull-up means HIGH is unpressed
uint32_t EncoderHandler::lastButtonDebounce = 0;

/**
 * @brief Quadrature state table.
 * Index is [old_A, old_B, new_A, new_B].
 * Returns -1, 0, or 1.
 */
static const int8_t ENC_STATES[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

void EncoderHandler::begin() {
    pinMode(PIN_ENC_CLK, INPUT_PULLUP);
    pinMode(PIN_ENC_DT, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    // Initial state
    lastState = (digitalRead(PIN_ENC_CLK) << 1) | digitalRead(PIN_ENC_DT);

    attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), handleISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_DT), handleISR, CHANGE);
}

void IRAM_ATTR EncoderHandler::handleISR() {
    uint8_t newState = (digitalRead(PIN_ENC_CLK) << 1) | digitalRead(PIN_ENC_DT);
    uint8_t index = (lastState << 2) | newState;
    encoderDelta += ENC_STATES[index & 0x0F];
    lastState = newState;
}

void EncoderHandler::run() {
    // 1. Process Rotation
    int8_t delta = 0;
    
    // Atomic read/reset of delta
    noInterrupts();
    delta = encoderDelta;
    if (delta >= 4 || delta <= -4) {
        encoderDelta %= 4; // Keep the remainder
    } else {
        delta = 0; // Not enough for a detent
    }
    interrupts();

    if (delta >= 4) {
        EventQueue::enqueue({EventType::EncoderCW, 0, 0, millis()});
    } else if (delta <= -4) {
        EventQueue::enqueue({EventType::EncoderCCW, 0, 0, millis()});
    }

    // 2. Process Button (Polled Debounce)
    bool currentButton = digitalRead(PIN_ENC_SW);
    uint32_t now = millis();

    if (currentButton != lastButtonState) {
        if (now - lastButtonDebounce > BUTTON_DEBOUNCE_MS) {
            lastButtonState = currentButton;
            lastButtonDebounce = now;
            
            if (currentButton == LOW) { // Pressed
                EventQueue::enqueue({EventType::EncoderButton, 0, 0, now});
            }
        }
    } else {
        lastButtonDebounce = now;
    }
}
