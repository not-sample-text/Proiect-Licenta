# Quick Reference: Modular Firmware Structure

## Files at a Glance

### Configuration

- **`include/config.h`** - All hardware pins, timing constants, matrix layout (~60 lines)
    - No executable code, just const definitions
    - Import this in any handler that needs pins/timings

### Entry Point

- **`src/main.cpp`** - The thin main entry (15 lines)
    ```cpp
    SystemOrganizer app;
    void setup() { app.begin(); }
    void loop() { app.run(); }
    ```

### Handlers (in `include/handlers/`)

| Handler                | Purpose                | Key Methods                           |
| ---------------------- | ---------------------- | ------------------------------------- |
| `debug_logger.h`       | Serial + OLED logging  | `log()`, `getMessage()`               |
| `battery_manager.h`    | MAX17048 I2C reading   | `getPercent()`, `getVoltageMv()`      |
| `matrix_handler.h`     | 4x3 keypad scanning    | `run()`, `getLastPressedKey()`        |
| `input_manager.h`      | Encoder/buttons/VBUS   | `processEncoder()`, `isVbusPresent()` |
| `status_led_manager.h` | WS2812B RGB LED        | `run(now, bleState, batteryPercent)`  |
| `display_manager.h`    | SSD1306 OLED rendering | `update()`, `nextScreen()`            |

### Coordinator

- **`include/system_organizer.h`** - Main app manager (~280 lines)
    - Orchestrates all handlers
    - Manages initialization state machine
    - Implements main loop logic with clear separation

### Implementation

- **`src/input_manager.cpp`** - Static member initialization (3 lines)
    - Required for ISR static pointer

## How to Use

### In your code:

```cpp
// Automatically handled by SystemOrganizer
// Just call begin() and run():

void setup() {
  Serial.begin(115200);
  app.begin();  // ← All subsystems initialize here
}

void loop() {
  app.run();    // ← Handles everything
}
```

### To add logging anywhere:

```cpp
// In your handler constructor:
MyHandler(DebugLogger& logger) : logger_(logger) {}

// Use it:
logger_.log("ERR: Something failed");
logger_.log("VAL: %u", someValue);
```

### To read battery:

```cpp
uint8_t percent;
uint16_t mv;
if (battery_.getPercent(percent)) {
  logger_.log("BAT: %u%%", percent);
}
```

### To check encoder:

```cpp
if (inputs_.isEncoderInterruptPending()) {
  int8_t rotation = inputs_.processEncoder(true);
  if (rotation > 0) {
    // Rotated clockwise
  } else if (rotation < 0) {
    // Rotated counter-clockwise
  }
}
```

### To navigate screens:

```cpp
display_.nextScreen();      // Move to next
display_.previousScreen();  // Move to previous
Screen current = display_.getCurrentScreen();
```

## Initialization Order

```
main.cpp setup()
  ↓
SystemOrganizer::begin()
  ├─ Enable power (LDO2)
  ├─ Initialize serial logger
  ├─ Start I2C bus
  ├─ Initialize StatusLedManager
  ├─ Initialize MatrixHandler
  ├─ Initialize InputManager
  └─ Initialize DisplayManager
      ├─ Uses DebugLogger
      ├─ Uses MatrixHandler
      ├─ Uses BatteryManager
      └─ Uses InputManager
```

## Runtime Loop Flow

```
main.cpp loop()
  ↓
SystemOrganizer::run()
  ├─ [Startup Phase] State machine:
  │  ├─ Wait 1s
  │  ├─ Init I2C & GPIO
  │  ├─ Init OLED display
  │  └─ Show boot flash for 2s
  │
  └─ [Running Phase] runNormal(now):
     ├─ Check power button (2s hold → sleep)
     ├─ Scan matrix @ 2ms
     ├─ Process encoder if interrupted
     ├─ Read inputs @ 150ms or on change
     ├─ Update display on state change
     └─ Update LED @ 50ms
```

## Handler Responsibilities

### DebugLogger

- Maintains circular buffer of last 4 messages
- Sends all messages to Serial immediately
- Provides messages to DisplayManager for debug screen

### BatteryManager

- Reads MAX17048 via I2C
- Converts raw values to percentage and voltage
- Returns false if chip unreachable

### MatrixHandler

- Scans 4×3 matrix with row-drive/column-sense
- Applies 5ms debounce per key
- Tracks which key is currently pressed

### InputManager

- Reads button digital states (non-blocking)
- Detects VBUS presence
- Processes encoder rotation with transition table
- Manages interrupt requests via static ISR

### StatusLedManager

- Controls WS2812B single LED
- Implements color logic based on system state
- Syncs with battery and BLE state

### DisplayManager

- Renders 4 different OLED screens
- Calls other handlers to gather display data
- Manages screen navigation

### SystemOrganizer

- Coordinates all handlers
- Manages initialization timing
- Implements main loop business logic
- Detects sleep trigger (2s encoder hold)

## Adding a New Handler

1. Create `include/handlers/my_handler.h`:

```cpp
#pragma once
#include "config.h"
#include "handlers/debug_logger.h"

class MyHandler {
 public:
  MyHandler(DebugLogger& logger) : logger_(logger) {}

  void begin() {
    logger_.log("INIT: MyHandler");
    // Your initialization
  }

  void run() {
    // Your periodic work
  }

 private:
  DebugLogger& logger_;
  // Your state variables
};
```

2. In `system_organizer.h`, add:

```cpp
MyHandler my_handler_;  // Add member

// In begin():
my_handler_.begin();

// In runNormal():
if ((now - last_my_update_ms_) >= UPDATE_PERIOD) {
  my_handler_.run();
  last_my_update_ms_ = now;
}
```

3. Recompile and test!

## Debugging Tips

- Check `ARCHITECTURE.md` for detailed component design
- Check `MIGRATION_GUIDE.md` for before/after comparisons
- Use the Debug screen (4th screen, rotate with encoder) to see logs
- All handlers log initialization via `logger_.log()`
- Each handler is independent - test them separately

## Performance Characteristics

- **Matrix scan**: Every 2ms (non-blocking)
- **Encoder check**: On interrupt (immediate)
- **Input refresh**: Every 150ms or on change
- **LED update**: Every 50ms
- **Display update**: Only on state change
- **Battery read**: On display update (~250ms typical)
- **Total loop**: ~5-10ms per iteration at 100% load

No blocking calls in normal operation. All timing is event-driven or periodic.
