#include "EncoderHandler.h"
#include "EventQueue.h"
#include "BoardSupport.h"
#include "OledHandler.h"

// Define pins exactly from the true hardware baseline
static constexpr uint8_t kEncoderClkPin = 2;
static constexpr uint8_t kEncoderDtPin = 4;
static constexpr uint8_t kEncoderSwPin = 1;

// Global interrupt flag mapped to memory-safe standalone IRAM tracking
volatile bool g_encoder_interrupt_pending = false;

void IRAM_ATTR inputManagerEncoderISR() {
    g_encoder_interrupt_pending = true;
}

uint8_t EncoderHandler::encoderLastState = 0;
int8_t EncoderHandler::encoderAccumulatedSteps = 0;
bool EncoderHandler::lastButtonState = true; 
uint32_t EncoderHandler::lastButtonDebounce = 0;
uint32_t EncoderHandler::buttonPressStartTime = 0;
bool EncoderHandler::shutdownTriggered = false;

uint8_t EncoderHandler::readRawState() {
    uint8_t clk = digitalRead(kEncoderClkPin) ? 1 : 0;
    uint8_t dt = digitalRead(kEncoderDtPin) ? 1 : 0;
    return static_cast<uint8_t>((clk << 1) | dt);
}

void EncoderHandler::begin() {
    pinMode(kEncoderClkPin, INPUT_PULLUP);
    pinMode(kEncoderDtPin, INPUT_PULLUP);
    pinMode(kEncoderSwPin, INPUT_PULLUP);

    // Seed the initial configuration
    encoderLastState = readRawState();

    attachInterrupt(digitalPinToInterrupt(kEncoderClkPin), inputManagerEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(kEncoderDtPin), inputManagerEncoderISR, CHANGE);
}

void EncoderHandler::run() {
    uint32_t now = millis();

    // 1. Process Rotation via the working deferred lookup logic
    if (g_encoder_interrupt_pending) {
        static constexpr int8_t kTransitionTable[16] = {
            0, 1, -1, 0,
            -1, 0, 0, 1,
            1, 0, 0, -1,
            0, -1, 1, 0,
        };

        uint8_t currentState = readRawState();
        int8_t transition = kTransitionTable[(encoderLastState << 2) | currentState];
        encoderLastState = currentState;

        if (transition != 0) {
            encoderAccumulatedSteps += transition;

            // Retain the threshold of 2 exactly from the tested to-migrate files
            if (encoderAccumulatedSteps >= 2) {
                encoderAccumulatedSteps -= 2;
                EventQueue::enqueue({EventType::EncoderCW, 0, 0, now});
            } else if (encoderAccumulatedSteps <= -2) {
                encoderAccumulatedSteps += 2;
                EventQueue::enqueue({EventType::EncoderCCW, 0, 0, now});
            }
        }
        g_encoder_interrupt_pending = false;
    }

// 2. Process Button with 2-second hold-to-shutdown feature
    bool currentButton = digitalRead(kEncoderSwPin);

    if (currentButton != lastButtonState) {
        if ((now - lastButtonDebounce) > BUTTON_DEBOUNCE_MS) {
            lastButtonState = currentButton;
            lastButtonDebounce = now;

            // FIX: Force the OLED to refresh its display buffers instantly 
            // so the INPUTS screen reflects the live button state change!
            OledHandler::update(); 

            if (currentButton == LOW) { // Button Pressed down
                buttonPressStartTime = now;
                shutdownTriggered = false;
            } else { // Button Released
                if (!shutdownTriggered) {
                    EventQueue::enqueue({EventType::EncoderButton, 0, 0, now});
                }
            }
        }
    } else if (currentButton == LOW && !shutdownTriggered) {
        // Evaluate hold length if button continues to be held down
        if ((now - buttonPressStartTime) >= SHUTDOWN_HOLD_MS) {
            shutdownTriggered = true;
            // Execute shutdown-mode sleep sequence (wake up only when VBUS goes high)
            BoardSupport::enterDeepSleep(true);
        }
    } else {
        lastButtonDebounce = now;
    }
}

bool EncoderHandler::isInterruptPending() {
    return g_encoder_interrupt_pending;
}
