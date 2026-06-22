#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <cstdint>
#include "config.h"
#include "handlers/debug_logger.h"
#include "handlers/battery_manager.h"
#include "handlers/status_led_manager.h"
#include "handlers/matrix_handler.h"
#include "handlers/input_manager.h"
#include "handlers/display_manager.h"

/**
 * Startup state enumeration
 */
enum class StartupState : uint8_t {
  kDelayBeforeInit = 0,
  kDelayBeforeOled = 1,
  kShowingBootFlash = 2,
  kRunning = 3,
};

/**
 * SystemOrganizer: Main coordinator that manages all system handlers
 * Entry points: begin() for initialization, run() for main loop
 */
class SystemOrganizer {
 public:
  SystemOrganizer()
      : logger_(),
        battery_(logger_),
        status_led_(),
        matrix_(logger_),
        inputs_(logger_),
        display_(logger_, matrix_, battery_, inputs_) {}

  /**
   * Initialize all subsystems in correct order
   */
  void begin() {
    // Enable power
    pinMode(kLdo2EnablePin, OUTPUT);
    digitalWrite(kLdo2EnablePin, HIGH);
    
    logger_.begin();
    logger_.log("SYS: Boot, LDO2 High");

    // Start I2C (needed for display and battery)
    Wire.begin(kOledSdaPin, kOledSclPin);

    // Initialize handlers in dependency order
    status_led_.begin();
    matrix_.begin();
    inputs_.begin();
    display_.begin();

    startup_state_ = StartupState::kDelayBeforeInit;
    state_timer_ms_ = millis();
  }

  /**
   * Main loop - handles initialization state machine and runtime loop
   */
  void run() {
    uint32_t now = millis();

    // Initialization state machine
    if (startup_state_ == StartupState::kDelayBeforeInit) {
      if ((now - state_timer_ms_) >= kInitDelayMs) {
        logger_.log("INIT: I2C & IO");
        status_led_.setBootState(1);
        state_timer_ms_ = now;
        startup_state_ = StartupState::kDelayBeforeOled;
      }
      return;
    }

    if (startup_state_ == StartupState::kDelayBeforeOled) {
      if ((now - state_timer_ms_) >= kOledInitDelayMs) {
        logger_.log("INIT: OLED Display");
        state_timer_ms_ = now;
        startup_state_ = StartupState::kShowingBootFlash;
      }
      return;
    }

    if (startup_state_ == StartupState::kShowingBootFlash) {
      // Run animated boot screen (takes ~1 second)
      if (!boot_animation_started_) {
        boot_animation_started_ = true;
        display_.animatedBootScreen();
        state_timer_ms_ = now;
      }
      
      // Wait additional time after animation for full 2-second boot splash
      if ((now - state_timer_ms_) >= (kBootFlashDurationMs - 1000)) {
        display_.update();
        logger_.log("SYS: Setup Complete");
        startup_state_ = StartupState::kRunning;
        boot_animation_started_ = false;
      }
      return;
    }

    // Normal runtime loop
    runNormal(now);
  }

  /**
   * Trigger sleep mode
   */
  void goToSleep() {
    logger_.log("SYS: Sleep Triggered");
    
    display_.animatedSleepScreen();
    status_led_.run(millis(), BleState::kDisconnected, 100);

    // Wait for button release
    while (inputs_.isEncoderSwitchPressed()) {
      delay(10);
    }
    delay(50);

    logger_.log("SYS: Ejecting USB...");
    Serial.end(); 
    delay(150);

    // Configure wakeup on VBUS
    esp_sleep_enable_ext1_wakeup(1ULL << kVbusSensePin, ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_deep_sleep_start();
  }

 private:
  // Subsystem handlers
  DebugLogger logger_;
  BatteryManager battery_;
  StatusLedManager status_led_;
  MatrixHandler matrix_;
  InputManager inputs_;
  DisplayManager display_;

  // State tracking
  StartupState startup_state_;
  uint32_t state_timer_ms_;
  bool boot_animation_started_ = false;

  // Runtime timers
  uint32_t last_matrix_scan_ms_ = 0;
  uint32_t last_encoder_process_ms_ = 0;
  uint32_t last_inputs_refresh_ms_ = 0;
  uint32_t last_led_update_ms_ = 0;

  // Input state tracking
  bool last_bt_select_ = false;
  bool last_encoder_switch_ = false;
  bool last_vbus_present_ = false;
  Screen last_drawn_screen_ = Screen::kInputs;

  // Power button tracking
  uint32_t encoder_hold_start_ms_ = 0;
  bool is_encoder_held_ = false;

  // System state
  BleState ble_state_ = BleState::kDisconnected;
  uint8_t cached_battery_percent_ = 100;

  void runNormal(uint32_t now) {
    // Check for power button hold (2 seconds to sleep)
    if (inputs_.isEncoderSwitchPressed()) {
      if (!is_encoder_held_) {
        is_encoder_held_ = true;
        encoder_hold_start_ms_ = now;
      } else if ((now - encoder_hold_start_ms_) >= kEncoderHoldDurationMs) {
        goToSleep();
      }
    } else {
      is_encoder_held_ = false;
    }

    // Scan matrix keypad periodically
    bool should_update = false;
    if ((now - last_matrix_scan_ms_) >= 2) {
      matrix_.run();
      should_update = true;
      last_matrix_scan_ms_ = now;
    }

    // Process encoder if interrupt pending
    if (inputs_.isEncoderInterruptPending() && (now - last_encoder_process_ms_) >= 2) {
      int8_t rotation = inputs_.processEncoder(true);
      if (rotation > 0) {
        display_.nextScreen();
      } else if (rotation < 0) {
        display_.previousScreen();
      }
      last_encoder_process_ms_ = now;
      should_update = true;
    }

    // Update digital inputs periodically or on change
    bool bt_select = inputs_.isBtSelectPressed();
    bool encoder_switch = inputs_.isEncoderSwitchPressed();
    bool vbus_present = inputs_.isVbusPresent();

    if ((now - last_inputs_refresh_ms_) >= 150 || 
        bt_select != last_bt_select_ ||
        encoder_switch != last_encoder_switch_ || 
        vbus_present != last_vbus_present_) {
      last_inputs_refresh_ms_ = now;
      last_bt_select_ = bt_select;
      last_encoder_switch_ = encoder_switch;
      
      if (vbus_present != last_vbus_present_) {
        logger_.log("PWR: VBUS %s", vbus_present ? "Connected" : "Disconnected");
        last_vbus_present_ = vbus_present;
      }
      
      if (!battery_.getPercent(cached_battery_percent_)) {
        static bool fuel_gauge_err_logged = false;
        if (!fuel_gauge_err_logged) {
          logger_.log("ERR: MAX17048G N/A");
          fuel_gauge_err_logged = true;
        }
      }
      
      should_update = true;
    }

    // Update display if needed
    if (should_update || display_.getCurrentScreen() != last_drawn_screen_) {
      display_.update();
      last_drawn_screen_ = display_.getCurrentScreen();
    }

    // Update status LED
    if ((now - last_led_update_ms_) >= 50) {
      status_led_.run(now, ble_state_, cached_battery_percent_);
      last_led_update_ms_ = now;
    }
  }
};
