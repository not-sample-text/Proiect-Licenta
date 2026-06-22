# Migration Guide: Monolithic → Modular

## Before: Monolithic Structure

**main.cpp** (~550 lines)

- Global constants scattered at top
- Global state variables (gEncoderInterruptPending, gCurrentScreen, gBleState, etc.)
- Global hardware objects (statusLed, display)
- Global arrays (gLogMessages, gMatrixStableState, etc.)
- Namespace wrapping all code except setup/loop
- Initialization state machine inline in loop()
- All handler functions global (scanMatrix(), processEncoder(), updateStatusLed(), etc.)
- Main loop with complex branching logic
- Hard to test individual components
- Hard to reuse code
- Risk of global state corruption

```cpp
// Old structure (simplified)
namespace {
  // ~100 global constants
  const uint8_t kOledAddress = 0x3C;
  // ...

  // ~20 global state variables
  volatile bool gEncoderInterruptPending = false;
  Screen gCurrentScreen = Screen::kMatrix;
  char gLogMessages[4][22] = {};
  // ...

  // Hardware objects
  Adafruit_NeoPixel statusLed(...);
  Adafruit_SSD1306 display(...);

  // ~20 free functions
  void logDebug(const char* format, ...);
  void configureInputs();
  void scanMatrix();
  void processEncoder();
  void updateStatusLed(uint32_t now);
  // ...
}

void setup() { /* complex init */ }
void loop() { /* 150+ lines */ }
```

**Problems:**

- ❌ Difficult to find where each variable is used
- ❌ Multiple systems interdependent through globals
- ❌ Initialization order is critical but implicit
- ❌ Hard to add new features without breaking existing code
- ❌ Impossible to test independently
- ❌ Code duplication across projects

---

## After: Modular Structure

**main.cpp** (5 lines)

```cpp
#include "system_organizer.h"

SystemOrganizer app;

void setup() { app.begin(); }
void loop() { app.run(); }
```

**System Organizer** manages:

- Initialization order
- Runtime coordination
- State transitions
- Component lifecycle

**Modular Components:**

```
config.h
├─ Centralized constants
└─ No code, just values

DebugLogger
├─ Circular log buffer
├─ Serial output
└─ Encapsulated state

BatteryManager
├─ I2C fuel gauge reading
├─ Voltage/percentage conversion
└─ Encapsulated I2C logic

MatrixHandler
├─ Keypad scanning
├─ Debouncing state machine
└─ Independent of display/logging

InputManager
├─ Button/VBUS state
├─ Encoder processing
├─ Interrupt handling (static instance)
└─ No global variables

DisplayManager
├─ OLED rendering
├─ 4 different screens
├─ Depends on: Logger, Matrix, Battery, Input
└─ Display state fully encapsulated

StatusLedManager
├─ WS2812B LED control
├─ Color state machine
└─ No external state
```

**Benefits:**

- ✅ Each component has single responsibility
- ✅ Dependencies explicitly shown in constructors
- ✅ Easy to find related code (all in one class)
- ✅ Components can be tested independently
- ✅ New features added without modifying existing code
- ✅ Reusable across projects
- ✅ Clear initialization order
- ✅ No global state corruption risk
- ✅ Easy to understand control flow

---

## Code Migration Examples

### Example 1: Debug Logging

**Before:**

```cpp
char gLogMessages[kMaxLogLines][kMaxLogLength] = {0};

void logDebug(const char* format, ...) {
  char buffer[kMaxLogLength];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  Serial.println(buffer);

  for (uint8_t i = 0; i < kMaxLogLines - 1; ++i) {
    strncpy(gLogMessages[i], gLogMessages[i + 1], kMaxLogLength);
  }
  strncpy(gLogMessages[kMaxLogLines - 1], buffer, kMaxLogLength);
}

// Usage in display code:
void oledShowDebugScreen() {
  for (uint8_t i = 0; i < kMaxLogLines; ++i) {
    display.setCursor(0, i * 8);
    display.print(gLogMessages[i]);
  }
  display.display();
}
```

**After:**

```cpp
class DebugLogger {
 private:
  char messages_[kMaxLogLines][kMaxLogLength] = {0};

 public:
  void log(const char* format, ...) { /* same logic */ }
  const char* getMessage(uint8_t index) const {
    return messages_[index];
  }
};

// Usage in display code:
void showDebugScreen() {
  for (uint8_t i = 0; i < kMaxLogLines; ++i) {
    display_.setCursor(0, i * 8);
    display_.print(logger_.getMessage(i));
  }
  display_.display();
}
```

### Example 2: Matrix Scanning

**Before:**

```cpp
bool gMatrixStableState[kMatrixRowCount][kMatrixColumnCount] = {};
bool gMatrixRawState[kMatrixRowCount][kMatrixColumnCount] = {};
uint32_t gMatrixLastChangeMs[kMatrixRowCount][kMatrixColumnCount] = {};
char gLastPressedKey = ' ';

bool scanMatrix() {
  uint32_t now = millis();
  bool changed = false;

  for (uint8_t row = 0; row < kMatrixRowCount; ++row) {
    // ... scanning logic ...
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

// Usage:
void loop() {
  if ((now - lastMatrixScanMs) >= 2) {
    shouldUpdate |= scanMatrix();
    lastMatrixScanMs = now;
  }

  char key = gLastPressedKey;
  // Use key somewhere...
}
```

**After:**

```cpp
class MatrixHandler {
 private:
  bool stable_state_[kMatrixRowCount][kMatrixColumnCount] = {};
  bool raw_state_[kMatrixRowCount][kMatrixColumnCount] = {};
  uint32_t last_change_ms_[kMatrixRowCount][kMatrixColumnCount] = {};
  char last_pressed_key_ = ' ';

 public:
  void run() {
    scan(); // All logic encapsulated
  }

  char getLastPressedKey() const {
    return last_pressed_key_;
  }
};

// Usage:
SystemOrganizer::runNormal() {
  if ((now - last_matrix_scan_ms_) >= 2) {
    matrix_.run();
    last_matrix_scan_ms_ = now;
  }

  char key = matrix_.getLastPressedKey();
  // Use key through clean interface...
}
```

### Example 3: System Organization

**Before:**

```cpp
void loop() {
  uint32_t now = millis();

  // Power button check
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

  // Initialization state machine
  if (gStartupState == StartupState::kDelayBeforeInit) {
    if ((now - gStateTimerMs) >= 1000) {
      logDebug("INIT: I2C & IO");
      Wire.begin(...);
      statusLed.begin();
      // ... more init
      gStateTimerMs = now;
      gStartupState = StartupState::kDelayBeforeOled;
    }
    return;
  }

  if (gStartupState == StartupState::kDelayBeforeOled) { /* ... */ }
  if (gStartupState == StartupState::kShowingBootFlash) { /* ... */ }

  // Normal operation...
  if ((now - lastMatrixScanMs) >= 2) { /* ... */ }
  if (gEncoderInterruptPending && (now - lastEncoderProcessMs) >= 2) { /* ... */ }
  // ... 50+ more lines
}
```

**After:**

```cpp
void loop() {
  app.run(); // That's it!
}

// Inside SystemOrganizer::run():
void run() {
  uint32_t now = millis();

  // Initialization state machine (abstracted)
  if (startup_state_ == StartupState::kDelayBeforeInit) { /* ... */ }
  if (startup_state_ == StartupState::kDelayBeforeOled) { /* ... */ }
  if (startup_state_ == StartupState::kShowingBootFlash) { /* ... */ }

  // Runtime (delegated to method)
  runNormal(now);
}

void runNormal(uint32_t now) {
  // Power button - clear and simple
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

  // Matrix scan - delegated
  if ((now - last_matrix_scan_ms_) >= 2) {
    matrix_.run(); // Let matrix handler do its thing
    last_matrix_scan_ms_ = now;
  }

  // Encoder - delegated
  if (inputs_.isEncoderInterruptPending() && (now - last_encoder_process_ms_) >= 2) {
    int8_t rotation = inputs_.processEncoder(true);
    if (rotation > 0) display_.nextScreen();
    else if (rotation < 0) display_.previousScreen();
    last_encoder_process_ms_ = now;
  }

  // ... rest is similarly clean
}
```

---

## Testing Benefits

### Old Way (Monolithic)

```cpp
// Can't test matrix scanning independently
// Can't test encoder without full hardware setup
// Can't test display without all other systems
// Must set up globals, test, tear down globals
```

### New Way (Modular)

```cpp
// Test matrix handler alone:
MatrixHandler matrix(mockLogger);
matrix.begin();
// Simulate button press...
assert(matrix.getLastPressedKey() == '5');

// Test encoder independently:
InputManager inputs(mockLogger);
inputs.begin();
// Simulate encoder rotation...
int8_t result = inputs.processEncoder(true);
assert(result == 1); // CW

// Test display with mock handlers:
MockBatteryManager battery;
MockInputManager input;
DisplayManager display(logger, matrix, battery, input);
display.begin();
// Test screen rendering...
```

---

## Adding New Features: Before vs After

### Scenario: Add Temperature Sensor

**Before (Monolithic):**

1. Add constants to main.cpp
2. Add global state variables to main.cpp
3. Write functions in global namespace
4. Modify global logDebug to handle temperature
5. Modify loop() to call new functions
6. Risk breaking existing functionality

**After (Modular):**

1. Create `TemperatureSensor` class in `handlers/temperature_sensor.h`
2. Implement `begin()` and `run()` methods
3. Add member to `SystemOrganizer`
4. Initialize in `SystemOrganizer::begin()`
5. Call in appropriate place in `runNormal()`
6. Existing code untouched - zero risk

```cpp
// Just add to SystemOrganizer:
TemperatureManager temp_;

void begin() {
  // ... existing init ...
  temp_.begin(); // Add one line
}

void runNormal(uint32_t now) {
  // ... existing code ...

  // Add new feature:
  if ((now - last_temp_update_ms_) >= 1000) {
    temp_.run();
    last_temp_update_ms_ = now;
  }
}
```

---

## Summary

| Aspect               | Monolithic    | Modular  |
| -------------------- | ------------- | -------- |
| main.cpp lines       | 550+          | 5        |
| Global state         | 20+ variables | 0        |
| Free functions       | 20+           | 0        |
| Component reuse      | ❌            | ✅       |
| Independent testing  | ❌            | ✅       |
| Adding features      | Risky         | Safe     |
| Code clarity         | Low           | High     |
| Maintainability      | Low           | High     |
| Initialization order | Implicit      | Explicit |
| Debugging            | Difficult     | Easy     |
