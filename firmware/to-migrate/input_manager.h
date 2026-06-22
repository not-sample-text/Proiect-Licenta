#pragma once

#include <Arduino.h>
#include <cstdint>
#include "config.h"
#include "handlers/debug_logger.h"

// Forward declarations
class InputManager;

// Global ISR handler - needed for ESP32 IRAM relocation safety
extern volatile bool g_encoder_interrupt_pending;

// Standalone interrupt handler (runs in IRAM)
void IRAM_ATTR inputManagerEncoderISR();

/**
 * InputManager: Handles button and encoder digital inputs
 * 
 * ENCODER DEBUG NOTES:
 * The encoder uses a quadrature decoder with transition table lookup.
 * Each full detent (click) should produce 4 transitions total.
 * If screens are switching every 2 detents, the accumulated_steps counter
 * may be triggering at wrong thresholds, or detents aren't aligned properly.
 */
class InputManager {
 public:
  InputManager(DebugLogger& logger) : logger_(logger) {}

  void begin() {
    configureInputs();
    configureInterrupts();
    encoder_last_state_ = readEncoderState();
  }

  void run() {
    // Input state is read on-demand during main loop
  }

  /**
   * Read VBUS present state
   */
  bool isVbusPresent() const {
    return digitalRead(kVbusSensePin) == HIGH;
  }

  /**
   * Read Bluetooth button state
   */
  bool isBtSelectPressed() const {
    return digitalRead(kBtSelectPin) == LOW;
  }

  /**
   * Read encoder switch state
   */
  bool isEncoderSwitchPressed() const {
    return digitalRead(kEncoderSwPin) == LOW;
  }

  /**
   * Process encoder rotation. Returns accumulated steps (positive=CW, negative=CCW)
   * Encoder steps are accumulated and returned when threshold is reached
   * 
   * DEBUG: Set kEncoderDebugLogging = true in config.h to see raw transitions
   */
  int8_t processEncoder(bool interrupt_pending) {
    if (!interrupt_pending) {
      return 0;
    }

    static constexpr int8_t kTransitionTable[16] = {
      0, 1, -1, 0,
      -1, 0, 0, 1,
      1, 0, 0, -1,
      0, -1, 1, 0,
    };

    uint8_t currentState = readEncoderState();
    int8_t transition = kTransitionTable[(encoder_last_state_ << 2) | currentState];
    
    // DEBUG: Log raw state transitions if enabled
    if constexpr (kEncoderDebugLogging) {
      if (transition != 0) {
        logger_.log("ENC: S%u->%u T:%d", encoder_last_state_, currentState, transition);
      }
    }
    
    encoder_last_state_ = currentState;

    if (transition == 0) {
      g_encoder_interrupt_pending = false;
      return 0;
    }

    encoder_accumulated_steps_ += transition;
    int8_t result = 0;

    if (encoder_accumulated_steps_ >= 2) {
      encoder_accumulated_steps_ -= 2;
      result = 1; // CW rotation
      if constexpr (kEncoderDebugLogging) {
        logger_.log("ENC: CW Step");
      }
    } else if (encoder_accumulated_steps_ <= -2) {
      encoder_accumulated_steps_ += 2;
      result = -1; // CCW rotation
      if constexpr (kEncoderDebugLogging) {
        logger_.log("ENC: CCW Step");
      }
    }

    g_encoder_interrupt_pending = false;
    return result;
  }

  /**
   * Check if encoder interrupt is pending
   */
  bool isEncoderInterruptPending() const {
    return g_encoder_interrupt_pending;
  }

  /**
   * DEBUG ACCESSOR: Get current accumulated steps count
   * Returns -1 to +1 range. When it reaches ±2, it triggers a step.
   */
  int8_t getEncoderAccumulatedSteps() const {
    return encoder_accumulated_steps_;
  }

  /**
   * DEBUG ACCESSOR: Get last state code (0-3)
   * Bit 1 = CLK, Bit 0 = DT
   */
  uint8_t getEncoderLastState() const {
    return encoder_last_state_;
  }

 private:
  DebugLogger& logger_;
  uint8_t encoder_last_state_ = 0;
  int8_t encoder_accumulated_steps_ = 0;

  void configureInputs() {
    pinMode(kBtSelectPin, INPUT_PULLUP);
    pinMode(kEncoderSwPin, INPUT_PULLUP);
    pinMode(kEncoderClkPin, INPUT_PULLUP);
    pinMode(kEncoderDtPin, INPUT_PULLUP);
    pinMode(kVbusSensePin, INPUT);
  }

  void configureInterrupts() {
    attachInterrupt(digitalPinToInterrupt(kEncoderClkPin), inputManagerEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(kEncoderDtPin), inputManagerEncoderISR, CHANGE);
  }

  uint8_t readEncoderState() const {
    uint8_t clk = digitalRead(kEncoderClkPin) ? 1 : 0;
    uint8_t dt = digitalRead(kEncoderDtPin) ? 1 : 0;
    return static_cast<uint8_t>((clk << 1) | dt);
  }
};
