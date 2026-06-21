#include <Arduino.h>
#include <Wire.h>
#include <cstdio>
#include <cstdarg>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace {

constexpr uint8_t kOledAddress = 0x3C;
constexpr uint8_t kOledWidth = 128;
constexpr uint8_t kOledHeight = 32;

constexpr uint8_t kOledSdaPin = 8;
constexpr uint8_t kOledSclPin = 9;

constexpr uint8_t kBtSelectPin = 34;
constexpr uint8_t kStatusLedPin = 18;
constexpr uint8_t kLdo2EnablePin = 17;

constexpr uint8_t kVbusSensePin = 21;
constexpr uint8_t kFuelGaugeAddress = 0x36;
constexpr uint8_t kFuelGaugeVcellRegister = 0x02;
constexpr uint8_t kFuelGaugeSocRegister = 0x04;

constexpr uint8_t kEncoderSwPin = 1;
constexpr uint8_t kEncoderClkPin = 2;
constexpr uint8_t kEncoderDtPin = 4;

constexpr uint8_t kAntennaPin = 11;

constexpr uint8_t kMatrixRowCount = 4;
constexpr uint8_t kMatrixColumnCount = 3;
constexpr uint8_t kMatrixColPins[kMatrixColumnCount] = {15, 37, 35};
constexpr uint8_t kMatrixRowPins[kMatrixRowCount] = {12, 13, 14, 5};
constexpr uint32_t kMatrixDebounceMs = 5;
constexpr char kMatrixKeyMap[kMatrixRowCount][kMatrixColumnCount] = {
  {'N', '#', '*'},
  {'7', '8', '9'},
  {'4', '5', '6'},
  {'1', '2', '3'},
};

enum class Screen : uint8_t {
 kMatrix = 0,
 kBattery = 1,
 kInputs = 2,
 kDebug = 3,  // <-- Added Debug Screen
};

enum class StartupState : uint8_t {
  kDelayBeforeInit = 0,
  kDelayBeforeOled = 1,
  kShowingBootFlash = 2,
  kRunning = 3,
};

volatile bool gEncoderInterruptPending = false;

StartupState gStartupState = StartupState::kDelayBeforeInit;
uint32_t gStateTimerMs = 0;

Screen gCurrentScreen = Screen::kMatrix;
char gLastPressedKey = ' ';
bool gMatrixStableState[kMatrixRowCount][kMatrixColumnCount] = {};
bool gMatrixRawState[kMatrixRowCount][kMatrixColumnCount] = {};
uint32_t gMatrixLastChangeMs[kMatrixRowCount][kMatrixColumnCount] = {};
uint8_t gEncoderLastState = 0;
int8_t gEncoderAccumulatedSteps = 0;

// ---------------------------------------------------------
// HARDWARE OBJECTS
// ---------------------------------------------------------

Adafruit_NeoPixel statusLed(1, kStatusLedPin, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(kOledWidth, kOledHeight, &Wire, -1);

// ---------------------------------------------------------
// DEBUG LOGGING SYSTEM
// ---------------------------------------------------------
constexpr uint8_t kMaxLogLines = 4;
constexpr uint8_t kMaxLogLength = 22; // 21 chars max at size 1 + null terminator
char gLogMessages[kMaxLogLines][kMaxLogLength] = {0};

void logDebug(const char* format, ...) {
  char buffer[kMaxLogLength];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  // Still send to actual hardware serial
  Serial.println(buffer);

  // Shift existing lines up the array
  for (uint8_t i = 0; i < kMaxLogLines - 1; ++i) {
    strncpy(gLogMessages[i], gLogMessages[i + 1], kMaxLogLength);
  }
  
  // Inject new line at the bottom
  strncpy(gLogMessages[kMaxLogLines - 1], buffer, kMaxLogLength);
  gLogMessages[kMaxLogLines - 1][kMaxLogLength - 1] = '\0'; 
}

// ---------------------------------------------------------
// LED & BLUETOOTH STATE TRACKING
// ---------------------------------------------------------
enum class BleState {
  kPairing,
  kJustConnected,
  kConnected,
  kDisconnected
};

BleState gBleState = BleState::kDisconnected;
uint32_t gBleConnectedTimerMs = 0;
uint8_t gCachedBatteryPercent = 100;

// ---------------------------------------------------------
// OLED HELPER FUNCTIONS
// ---------------------------------------------------------
void oledDrawTextCentered(int y, const char *text) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((kOledWidth - w) / 2, y);
  display.print(text);
}

void configureInputPullup(uint8_t pin) {
 pinMode(pin, INPUT_PULLUP);
}

void configureMatrixPins() {
 for (uint8_t row = 0; row < kMatrixRowCount; ++row) {
  pinMode(kMatrixRowPins[row], INPUT);
 }

 for (uint8_t col = 0; col < kMatrixColumnCount; ++col) {
  pinMode(kMatrixColPins[col], INPUT_PULLUP);
 }
}

void IRAM_ATTR encoderInterruptHandler() {
 gEncoderInterruptPending = true;
}

void configureInterrupts() {
 attachInterrupt(digitalPinToInterrupt(kEncoderClkPin), encoderInterruptHandler, CHANGE);
 attachInterrupt(digitalPinToInterrupt(kEncoderDtPin), encoderInterruptHandler, CHANGE);
}

void configureInputs() {
 configureInputPullup(kBtSelectPin);
 configureInputPullup(kEncoderSwPin);
 configureInputPullup(kEncoderClkPin);
 configureInputPullup(kEncoderDtPin);
 pinMode(kVbusSensePin, INPUT);
}

bool readVbusPresent() {
 return digitalRead(kVbusSensePin) == HIGH;
}

bool fuelGaugeRead16(uint8_t reg, uint16_t &value) {
 Wire.beginTransmission(kFuelGaugeAddress);
 Wire.write(reg);
 if (Wire.endTransmission(false) != 0) {
  return false;
 }

 if (Wire.requestFrom(static_cast<int>(kFuelGaugeAddress), 2) != 2) {
  return false;
 }

 uint8_t msb = static_cast<uint8_t>(Wire.read());
 uint8_t lsb = static_cast<uint8_t>(Wire.read());
 value = static_cast<uint16_t>((static_cast<uint16_t>(msb) << 8) | lsb);
 return true;
}

bool readFuelGaugePercent(uint8_t &percent) {
 uint16_t socRaw = 0;
 if (!fuelGaugeRead16(kFuelGaugeSocRegister, socRaw)) {
  return false;
 }

 uint16_t whole = socRaw >> 8;
 uint8_t fractional = static_cast<uint8_t>(socRaw & 0xFF);
 uint16_t rounded = whole + ((fractional >= 128) ? 1 : 0);
 if (rounded > 100) {
  rounded = 100;
 }

 percent = static_cast<uint8_t>(rounded);
 return true;
}

bool readFuelGaugeVoltageMv(uint16_t &voltageMv) {
 uint16_t vcellRaw = 0;
 if (!fuelGaugeRead16(kFuelGaugeVcellRegister, vcellRaw)) {
  return false;
 }

 uint64_t millivolts = (static_cast<uint64_t>(vcellRaw) * 78125ULL + 500000ULL) / 1000000ULL;
 voltageMv = static_cast<uint16_t>(millivolts);
 return true;
}

bool scanMatrix() {
  uint32_t now = millis();
  bool changed = false;

  for (uint8_t row = 0; row < kMatrixRowCount; ++row) {
    pinMode(kMatrixRowPins[row], OUTPUT);
    digitalWrite(kMatrixRowPins[row], LOW);
    
    delayMicroseconds(100);

    for (uint8_t col = 0; col < kMatrixColumnCount; ++col) {
      bool pressed = digitalRead(kMatrixColPins[col]) == LOW;

      if (pressed != gMatrixRawState[row][col]) {
        gMatrixRawState[row][col] = pressed;
        gMatrixLastChangeMs[row][col] = now;
      }

      if ((now - gMatrixLastChangeMs[row][col]) >= kMatrixDebounceMs && gMatrixStableState[row][col] != gMatrixRawState[row][col]) {
        gMatrixStableState[row][col] = gMatrixRawState[row][col];
        changed = true;
      }
    }

    pinMode(kMatrixRowPins[row], INPUT);

    for (uint8_t col = 0; col < kMatrixColumnCount; ++col) {
      pinMode(kMatrixColPins[col], OUTPUT);
      digitalWrite(kMatrixColPins[col], HIGH);
    }
    
    delayMicroseconds(10); 
    
    for (uint8_t col = 0; col < kMatrixColumnCount; ++col) {
      pinMode(kMatrixColPins[col], INPUT_PULLUP);
    }
  }

  if (changed) {
    for (uint8_t r = 0; r < kMatrixRowCount; ++r) {
      for (uint8_t c = 0; c < kMatrixColumnCount; ++c) {
        if (gMatrixStableState[r][c]) {
          gLastPressedKey = kMatrixKeyMap[r][c];
          logDebug("Matrix: Key '%c' Active", gLastPressedKey);
        }
      }
    }
  }

  return changed;
}

uint8_t readEncoderState() {
 uint8_t clk = digitalRead(kEncoderClkPin) ? 1 : 0;
 uint8_t dt = digitalRead(kEncoderDtPin) ? 1 : 0;
 return static_cast<uint8_t>((clk << 1) | dt);
}

void processEncoder() {
 static constexpr int8_t kTransitionTable[16] = {
   0, 1, -1, 0,
   -1, 0, 0, 1,
   1, 0, 0, -1,
   0, -1, 1, 0,
 };

 uint8_t currentState = readEncoderState();
 int8_t transition = kTransitionTable[(gEncoderLastState << 2) | currentState];
 gEncoderLastState = currentState;

 if (transition == 0) {
  gEncoderInterruptPending = false;
  return;
 }

 gEncoderAccumulatedSteps += transition;

 if (gEncoderAccumulatedSteps >= 4) {
  gEncoderAccumulatedSteps = 0;
  // Modulo updated to 4 to include the new Debug screen
  gCurrentScreen = static_cast<Screen>((static_cast<uint8_t>(gCurrentScreen) + 1) % 4);
  logDebug("UI: Next Screen");
 } else if (gEncoderAccumulatedSteps <= -4) {
  gEncoderAccumulatedSteps = 0;
  gCurrentScreen = static_cast<Screen>((static_cast<uint8_t>(gCurrentScreen) + 3) % 4);
  logDebug("UI: Prev Screen");
 }

 gEncoderInterruptPending = false;
}

void oledShowMatrixScreen() {
 char keyLine[16];
 snprintf(keyLine, sizeof(keyLine), "KEY %c", gLastPressedKey == ' ' ? '-' : gLastPressedKey);

 display.clearDisplay();
 display.setTextSize(1);
 display.setTextColor(SSD1306_WHITE);
 oledDrawTextCentered(0, "MATRIX");
 oledDrawTextCentered(16, keyLine);
 display.display();
}

void oledShowBatteryScreen() {
 uint8_t batteryPercent = 0;
 uint16_t batteryVoltageMv = 0;
 bool percentOk = readFuelGaugePercent(batteryPercent);
 bool voltageOk = readFuelGaugeVoltageMv(batteryVoltageMv);

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

 display.clearDisplay();
 display.setTextSize(1);
 display.setTextColor(SSD1306_WHITE);
 oledDrawTextCentered(0, "BATTERY");
 oledDrawTextCentered(12, stateLine);
 oledDrawTextCentered(22, voltageLine);
 display.display();
}

void oledShowInputsScreen() {
 bool btSelect = digitalRead(kBtSelectPin) == LOW;
 bool vbusPresent = readVbusPresent();
 bool encSwitch = digitalRead(kEncoderSwPin) == LOW;

 char btLine[16];
 snprintf(btLine, sizeof(btLine), "BT %u", btSelect ? 1U : 0U);

 char vbusLine[16];
 snprintf(vbusLine, sizeof(vbusLine), "VBUS %u", vbusPresent ? 1U : 0U);

 char encLine[16];
 snprintf(encLine, sizeof(encLine), "ENC %u", encSwitch ? 1U : 0U);

 display.clearDisplay();
 display.setTextSize(1);
 display.setTextColor(SSD1306_WHITE);
 oledDrawTextCentered(0, "INPUTS");
 display.setCursor(0, 10); display.print(btLine);
 display.setCursor(0, 18); display.print(vbusLine);
 display.setCursor(0, 26); display.print(encLine);
 display.display();
}

void oledShowDebugScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  for (uint8_t i = 0; i < kMaxLogLines; ++i) {
    display.setCursor(0, i * 8); // 8 pixels per line
    display.print(gLogMessages[i]);
  }
  
  display.display();
}

void updateOled() {
 switch (gCurrentScreen) {
  case Screen::kMatrix:
   oledShowMatrixScreen();
   break;
  case Screen::kBattery:
   oledShowBatteryScreen();
   break;
  case Screen::kInputs:
   oledShowInputsScreen();
   break;
  case Screen::kDebug:
   oledShowDebugScreen();
   break;
 }
}

} // namespace

void setup() {
 Serial.begin(115200);
 gStateTimerMs = millis();

 pinMode(kLdo2EnablePin, OUTPUT);
 digitalWrite(kLdo2EnablePin, HIGH);
 
 logDebug("SYS: Boot, LDO2 High");
}

void goToSleep() {
  logDebug("SYS: Sleep Triggered");
  
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);

  statusLed.clear();
  statusLed.show();

  while (digitalRead(kEncoderSwPin) == LOW) {
    delay(10);
  }
  delay(50); 

  logDebug("SYS: Ejecting USB...");
  Serial.end(); 
  delay(150);   

  esp_sleep_enable_ext1_wakeup(1ULL << kVbusSensePin, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_deep_sleep_start();
}

void updateStatusLed(uint32_t now) {
  // THE FIX: Do not attempt to update the RMT driver until initialized
  if (gStartupState < StartupState::kDelayBeforeOled) {
    return;
  }

  static uint32_t lastLedBlinkMs = 0;
  static bool blinkState = false;
  
  if ((now - lastLedBlinkMs) >= 250) {
    blinkState = !blinkState;
    lastLedBlinkMs = now;
  }

  if (gBleState == BleState::kPairing) {
    if (blinkState) {
      statusLed.setPixelColor(0, statusLed.Color(0, 0, 255));
    } else {
      statusLed.setPixelColor(0, statusLed.Color(0, 0, 0));
    }
    statusLed.show();
    return;
  }

  if (gBleState == BleState::kJustConnected) {
    statusLed.setPixelColor(0, statusLed.Color(0, 0, 255));
    statusLed.show();
    
    if ((now - gBleConnectedTimerMs) > 7500) {
      gBleState = BleState::kConnected;
    }
    return;
  }

  if (gCachedBatteryPercent <= 20) {
    bool slowBlink = ((now / 500) % 2) == 0;
    if (slowBlink) {
      statusLed.setPixelColor(0, statusLed.Color(255, 0, 0));
    } else {
      statusLed.setPixelColor(0, statusLed.Color(0, 0, 0));
    }
    statusLed.show();
    return;
  }

  uint8_t red = 0;
  uint8_t green = 0;

  if (gCachedBatteryPercent >= 60) {
    red = map(gCachedBatteryPercent, 60, 100, 255, 0);
    green = 255;
  } else {
    red = 255;
    green = map(gCachedBatteryPercent, 21, 59, 0, 255);
  }

  statusLed.setPixelColor(0, statusLed.Color(red, green, 0));
  statusLed.show();
}

void loop() {
 uint32_t now = millis();

 static uint32_t encoderHoldStartMs = 0;
 static bool isEncoderHeld = false;

 if (digitalRead(kEncoderSwPin) == LOW) {
   if (!isEncoderHeld) {
     isEncoderHeld = true;
     encoderHoldStartMs = now;
   } else if ((now - encoderHoldStartMs) >= 2000) { 
     goToSleep(); 
   }
 } else {
   isEncoderHeld = false;
 }

 // ---------------------------------------------------------
 // Non-Blocking Initialization State Machine
 // ---------------------------------------------------------
 if (gStartupState == StartupState::kDelayBeforeInit) {
  if ((now - gStateTimerMs) >= 1000) {
   logDebug("INIT: I2C & IO");
   Wire.begin(kOledSdaPin, kOledSclPin);

   statusLed.begin();
   statusLed.setBrightness(255);
   statusLed.show();
   logDebug("INIT: WS2812B OK");

   configureMatrixPins();
   configureInputs();
   configureInterrupts();

   gEncoderLastState = readEncoderState();

   gStateTimerMs = now;
   gStartupState = StartupState::kDelayBeforeOled;
  }
  return;
 }

 if (gStartupState == StartupState::kDelayBeforeOled) {
  if ((now - gStateTimerMs) >= 100) {
   logDebug("INIT: OLED Display");
   if(!display.begin(SSD1306_SWITCHCAPVCC, kOledAddress)) {
     logDebug("ERR: OLED Failed");
   }
   display.clearDisplay();
   display.display();

   gStateTimerMs = now;
   gStartupState = StartupState::kShowingBootFlash;
  }
  return;
 }

 if (gStartupState == StartupState::kShowingBootFlash) {
  if ((now - gStateTimerMs) >= 2000) {
   display.clearDisplay();
   display.display();
   updateOled();

   logDebug("SYS: Setup Complete");
   gStartupState = StartupState::kRunning;
  }
  return;
 }

 // ---------------------------------------------------------
 // Normal Loop Execution (Running State)
 // ---------------------------------------------------------
 static uint32_t lastMatrixScanMs = 0;
 static uint32_t lastEncoderProcessMs = 0;
 static uint32_t lastInputsRefreshMs = 0;
 static bool lastBtSelect = false;
 static bool lastEncoderSwitch = false;
 static bool lastVbusPresent = false;
 static Screen lastDrawnScreen = Screen::kInputs;

 bool shouldUpdate = false;

 if ((now - lastMatrixScanMs) >= 2) {
  shouldUpdate |= scanMatrix();
  lastMatrixScanMs = now;
 }

 if (gEncoderInterruptPending && (now - lastEncoderProcessMs) >= 2) {
  processEncoder();
  lastEncoderProcessMs = now;
  shouldUpdate = true;
 }

 bool btSelect = digitalRead(kBtSelectPin) == LOW;
 bool encoderSwitch = digitalRead(kEncoderSwPin) == LOW;
 bool vbusPresent = readVbusPresent();

 if ((now - lastInputsRefreshMs) >= 150 || btSelect != lastBtSelect ||
   encoderSwitch != lastEncoderSwitch || vbusPresent != lastVbusPresent) {
  lastInputsRefreshMs = now;
  lastBtSelect = btSelect;
  lastEncoderSwitch = encoderSwitch;
  
  if (vbusPresent != lastVbusPresent) {
    logDebug("PWR: VBUS %s", vbusPresent ? "Connected" : "Disconnected");
    lastVbusPresent = vbusPresent;
  }
  
  if (!readFuelGaugePercent(gCachedBatteryPercent)) {
     static bool fuelGaugeErrLogged = false;
     if (!fuelGaugeErrLogged) {
       logDebug("ERR: MAX17048G N/A");
       fuelGaugeErrLogged = true;
     }
  }
  
  shouldUpdate = true;
 }

 if (shouldUpdate || gCurrentScreen != lastDrawnScreen) {
  updateOled();
  lastDrawnScreen = gCurrentScreen;
 }

 updateStatusLed(now);
}
