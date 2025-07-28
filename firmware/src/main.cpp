#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RotaryEncoder.h>
#include <Keypad.h>
#include "Mylibrary/pin_config.h"

// --- Placeholder for future logic ---
// This is where you'll add the code to handle what happens when you
// press a key, turn the encoder, etc.

void setup() {
  // Start Serial for debugging
  Serial.begin(115200);
  Serial.println("Macro Pad Booting Up...");

  // Setup OLED display
  // (Code to initialize and clear the display will go here)

  // Setup Rotary Encoder
  // (Code to initialize the encoder will go here)

  // Setup Key Matrix
  // (Code to initialize the keypad will go here)

  // Setup Mode Switch
  pinMode(PIN_MODE_SWITCH, INPUT_PULLUP);

  Serial.println("Setup Complete.");
}

void loop() {
  // 1. Scan the key matrix for presses
  // (Code to check the keypad will go here)

  // 2. Check the rotary encoder for changes
  // (Code to check the encoder will go here)

  // 3. Update the display if anything changed
  // (Code to draw to the screen will go here)
}
