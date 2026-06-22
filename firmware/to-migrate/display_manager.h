#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <cstdint>
#include <cstdio>
#include "config.h"
#include "handlers/debug_logger.h"
#include "handlers/matrix_handler.h"
#include "handlers/battery_manager.h"
#include "handlers/input_manager.h"

/**
 * Screen enumeration
 */
enum class Screen : uint8_t {
  kMatrix = 0,
  kBattery = 1,
  kInputs = 2,
  kDebug = 3,
  kEncoderDebug = 4,  // NEW: Dedicated encoder raw input debug screen
};

/**
 * DisplayManager: Manages OLED display and screen rendering
 */
class DisplayManager {
 public:
  DisplayManager(DebugLogger& logger, MatrixHandler& matrix, 
                 BatteryManager& battery, InputManager& inputs)
      : logger_(logger), matrix_(matrix), battery_(battery), inputs_(inputs),
        display_(kOledWidth, kOledHeight, &Wire, -1) {}

  void begin() {
    if(!display_.begin(SSD1306_SWITCHCAPVCC, kOledAddress)) {
      logger_.log("ERR: OLED Failed");
    }
    display_.clearDisplay();
    display_.display();
  }

  void run() {
    // Display updates are triggered on demand by SystemOrganizer
  }

  /**
   * Update display with current screen
   */
  void update() {
    switch (current_screen_) {
      case Screen::kMatrix:
        showMatrixScreen();
        break;
      case Screen::kBattery:
        showBatteryScreen();
        break;
      case Screen::kInputs:
        showInputsScreen();
        break;
      case Screen::kDebug:
        showDebugScreen();
        break;
      case Screen::kEncoderDebug:
        showEncoderDebugScreen();
        break;
    }
  }

  /**
   * Rotate to next screen
   */
  void nextScreen() {
    uint8_t max_screens = 5; // Updated to include new encoder debug screen
    current_screen_ = static_cast<Screen>((static_cast<uint8_t>(current_screen_) + 1) % max_screens);
    logger_.log("UI: Next Screen");
  }

  /**
   * Rotate to previous screen
   */
  void previousScreen() {
    uint8_t max_screens = 5; // Updated to include new encoder debug screen
    current_screen_ = static_cast<Screen>((static_cast<uint8_t>(current_screen_) + max_screens - 1) % max_screens);
    logger_.log("UI: Prev Screen");
  }

  /**
   * Get current screen
   */
  Screen getCurrentScreen() const {
    return current_screen_;
  }

  /**
   * Clear display and turn off
   */
  void clear() {
    display_.clearDisplay();
    display_.display();
    display_.ssd1306_command(SSD1306_DISPLAYOFF);
  }

  /**
   * Show boot flash (full white screen for 2 seconds)
   */
  void showBootFlash() {
    display_.clearDisplay();
    display_.fillRect(0, 0, kOledWidth, kOledHeight, SSD1306_WHITE);
    display_.display();
  }

  /**
   * Animated boot screen - wipe from left to right filling with white
   * Takes approximately 1 second to complete
   */
  void animatedBootScreen() {
    const uint8_t wipeSteps = 20;
    const uint32_t stepDelayMs = 50; // 50ms per step = ~1 second total
    const uint8_t pixelsPerStep = kOledWidth / wipeSteps;

    display_.clearDisplay();
    
    for (uint8_t step = 0; step <= wipeSteps; ++step) {
      uint8_t fillWidth = (pixelsPerStep * step);
      if (fillWidth > kOledWidth) fillWidth = kOledWidth;
      
      display_.clearDisplay();
      display_.fillRect(0, 0, fillWidth, kOledHeight, SSD1306_WHITE);
      display_.display();
      
      if (step < wipeSteps) {
        delay(stepDelayMs);
      }
    }
  }

  /**
   * Animated sleep screen - wipe from left to right clearing to black
   * Takes approximately 1 second to complete
   */
  void animatedSleepScreen() {
    const uint8_t wipeSteps = 20;
    const uint32_t stepDelayMs = 50; // 50ms per step = ~1 second total
    const uint8_t pixelsPerStep = kOledWidth / wipeSteps;

    // Start with full white screen
    display_.clearDisplay();
    display_.fillRect(0, 0, kOledWidth, kOledHeight, SSD1306_WHITE);
    display_.display();
    delay(100); // Brief pause showing full white

    // Wipe from left to right, clearing to black
    for (uint8_t step = 0; step <= wipeSteps; ++step) {
      uint8_t remainingWidth = kOledWidth - (pixelsPerStep * step);
      if (remainingWidth < 0) remainingWidth = 0;
      
      display_.clearDisplay();
      display_.fillRect(pixelsPerStep * step, 0, remainingWidth, kOledHeight, SSD1306_WHITE);
      display_.display();
      
      if (step < wipeSteps) {
        delay(stepDelayMs);
      }
    }

    // Final blank screen
    display_.clearDisplay();
    display_.display();
  }

 private:
  DebugLogger& logger_;
  MatrixHandler& matrix_;
  BatteryManager& battery_;
  InputManager& inputs_;
  Adafruit_SSD1306 display_;
  Screen current_screen_ = Screen::kMatrix;

  void drawTextCentered(int y, const char *text) {
    int16_t x1, y1;
    uint16_t w, h;
    display_.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    display_.setCursor((kOledWidth - w) / 2, y);
    display_.print(text);
  }

  void showMatrixScreen() {
    char keyLine[16];
    char key = matrix_.getLastPressedKey();
    snprintf(keyLine, sizeof(keyLine), "KEY %c", key == ' ' ? '-' : key);

    display_.clearDisplay();
    display_.setTextSize(1);
    display_.setTextColor(SSD1306_WHITE);
    drawTextCentered(0, "MATRIX");
    drawTextCentered(16, keyLine);
    display_.display();
  }

  void showBatteryScreen() {
    uint8_t batteryPercent = 0;
    uint16_t batteryVoltageMv = 0;
    bool percentOk = battery_.getPercent(batteryPercent);
    bool voltageOk = battery_.getVoltageMv(batteryVoltageMv);

    char stateLine[16];
    if (percentOk) {
      snprintf(stateLine, sizeof(stateLine), "BAT %u%%", batteryPercent);
    } else {
      snprintf(stateLine, sizeof(stateLine), "BAT ERR");
    }

    char voltageLine[16];
    if (voltageOk) {
      uint16_t wholeVolts = batteryVoltageMv / 1000;
      uint16_t fracVolts = (batteryVoltageMv % 1000) / 10;
      snprintf(voltageLine, sizeof(voltageLine), "%u.%02uV", wholeVolts, fracVolts);
    } else {
      snprintf(voltageLine, sizeof(voltageLine), "0x36 N/A");
    }

    display_.clearDisplay();
    display_.setTextSize(1);
    display_.setTextColor(SSD1306_WHITE);
    drawTextCentered(0, "BATTERY");
    drawTextCentered(12, stateLine);
    drawTextCentered(22, voltageLine);
    display_.display();
  }

  void showInputsScreen() {
    bool btSelect = inputs_.isBtSelectPressed();
    bool vbusPresent = inputs_.isVbusPresent();
    bool encSwitch = inputs_.isEncoderSwitchPressed();

    char btLine[16];
    snprintf(btLine, sizeof(btLine), "BT %u", btSelect ? 1U : 0U);

    char vbusLine[16];
    snprintf(vbusLine, sizeof(vbusLine), "VBUS %u", vbusPresent ? 1U : 0U);

    char encLine[16];
    snprintf(encLine, sizeof(encLine), "ENC %u", encSwitch ? 1U : 0U);

    display_.clearDisplay();
    display_.setTextSize(1);
    display_.setTextColor(SSD1306_WHITE);
    drawTextCentered(0, "INPUTS");
    display_.setCursor(0, 10); display_.print(btLine);
    display_.setCursor(0, 18); display_.print(vbusLine);
    display_.setCursor(0, 26); display_.print(encLine);
    display_.display();
  }

  void showDebugScreen() {
    display_.clearDisplay();
    display_.setTextSize(1);
    display_.setTextColor(SSD1306_WHITE);
    
    for (uint8_t i = 0; i < kMaxLogLines; ++i) {
      display_.setCursor(0, i * 8);
      display_.print(logger_.getMessage(i));
    }
    
    display_.display();
  }

  /**
   * NEW: Show raw encoder state debug screen
   * Displays: CLK/DT raw pins, current state code, accumulated steps
   */
  void showEncoderDebugScreen() {
    // Get raw encoder states
    uint8_t clk = digitalRead(kEncoderClkPin) ? 1 : 0;
    uint8_t dt = digitalRead(kEncoderDtPin) ? 1 : 0;
    uint8_t rawState = (clk << 1) | dt;  // state code (0-3)
    
    // Get encoder internal state from input manager
    uint8_t accSteps = inputs_.getEncoderAccumulatedSteps();
    uint8_t lastState = inputs_.getEncoderLastState();

    char line1[22];
    char line2[22];
    char line3[22];

    // Format: CLK:X DT:X (raw pins)
    snprintf(line1, sizeof(line1), "CLK:%u DT:%u", clk, dt);
    
    // Format: State:X (0-3 representing the CLK/DT combination)
    snprintf(line2, sizeof(line2), "State:%u AccSteps:%d", rawState, accSteps);
    
    // Format: LastState:X
    snprintf(line3, sizeof(line3), "Last:%u Pend:%u", lastState, inputs_.isEncoderInterruptPending() ? 1U : 0U);

    display_.clearDisplay();
    display_.setTextSize(1);
    display_.setTextColor(SSD1306_WHITE);
    drawTextCentered(0, "ENC DEBUG");
    display_.setCursor(0, 10); display_.print(line1);
    display_.setCursor(0, 18); display_.print(line2);
    display_.setCursor(0, 26); display_.print(line3);
    display_.display();
  }
};
