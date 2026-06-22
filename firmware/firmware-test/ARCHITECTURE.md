# Modularized Firmware Architecture

This document describes the refactored firmware structure that transforms the monolithic `main.cpp` into a well-organized, modular system.

## Overview

The firmware now follows an **object-oriented, component-based architecture** where functionality is split into discrete handlers/managers, coordinated by a central `SystemOrganizer` class.

### Entry Points

- **`SystemOrganizer::begin()`** - Initializes all subsystems in dependency order during setup
- **`SystemOrganizer::run()`** - Main loop that handles initialization state machine and runtime operations

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────┐
│                    main.cpp (thin)                      │
│                                                         │
│  SystemOrganizer app;                                   │
│  void setup() { app.begin(); }                          │
│  void loop() { app.run(); }                             │
└────────────────────┬────────────────────────────────────┘
                     │
                     ▼
         ┌───────────────────────────┐
         │   SystemOrganizer         │
         │  (Coordinator/Manager)    │
         └───────────┬───────────────┘
                     │
         ┌───────────┼───────────────────┬───────────────┬───────────────┬──────────────┐
         │           │                   │               │               │              │
         ▼           ▼                   ▼               ▼               ▼              ▼
    ┌───────────┐ ┌───────────────┐ ┌──────────────┐ ┌────────────────┐ ┌─────────────┐ ┌──────────────┐
    │DebugLogger│ │ BatteryManager│ │DisplayManager│ │StatusLedManager│ │MatrixHandler│ │InputManager  │
    └───────────┘ └───────────────┘ └──────────────┘ └────────────────┘ └─────────────┘ └──────────────┘
```

## Component Breakdown

### 1. **config.h** - Central Configuration

- All hardware pin definitions
- Timing constants
- Matrix keypad layout
- Logging buffer sizes

**Key Constants:**

- Pin assignments (OLED, encoder, matrix, LEDs, power)
- Timing values (debounce, blink rates, timeouts)
- Matrix keymap

### 2. **DebugLogger** - Debug Output Handler

**File:** `handlers/debug_logger.h`

Centralized logging to both serial and in-memory circular buffer displayed on OLED.

**Methods:**

- `begin()` - Initialize logger
- `run()` - (No-op for logger)
- `log(format, ...)` - Printf-style formatted logging
- `getMessage(index)` - Retrieve log line for display
- `clearLogs()` - Clear buffer

**Features:**

- Circular buffer of 4 lines × 22 characters
- Automatically rotates messages
- Sends to Serial immediately

### 3. **BatteryManager** - Fuel Gauge Reader

**File:** `handlers/battery_manager.h`

Reads battery percentage and voltage from MAX17048G fuel gauge via I2C.

**Methods:**

- `begin()` - Setup (uses Wire initialized by SystemOrganizer)
- `run()` - (Battery reads are on-demand)
- `getPercent(percent&)` - Read 0-100 battery percentage
- `getVoltageMv(voltage&)` - Read voltage in millivolts

**Implementation:**

- I2C register reads (SOC @ 0x04, VCELL @ 0x02)
- Proper byte order handling
- Error checking and fallback values

### 4. **MatrixHandler** - Keypad Scanner

**File:** `handlers/matrix_handler.h`

Scans 4×3 keypad matrix with hardware debouncing.

**Methods:**

- `begin()` - Configure matrix pins
- `run()` - Scan matrix for pressed keys
- `getLastPressedKey()` - Get current pressed key
- `isKeyPressed(row, col)` - Check individual button state

**Features:**

- Row-drive, column-sense scanning
- 5ms debounce per key
- Tracks raw and stable states
- Logs key press events

### 5. **InputManager** - Digital Inputs & Encoder

**File:** `handlers/input_manager.h`

Handles buttons, VBUS detection, and rotary encoder with interrupt processing.

**Methods:**

- `begin()` - Configure pins and interrupts
- `run()` - (Input state queried on-demand)
- `isVbusPresent()` - USB power detection
- `isBtSelectPressed()` - Bluetooth select button
- `isEncoderSwitchPressed()` - Encoder push button
- `processEncoder(interrupt_pending)` - Process encoder rotation
- `setEncoderInterruptPending()` - Called by ISR
- `isEncoderInterruptPending()` - Check for pending interrupt

**Features:**

- Interrupt-driven encoder processing
- State transition table for robust encoder decoding
- 4-step accumulation before registering rotation
- Separate thread-safe interrupt handler

### 6. **StatusLedManager** - RGB LED Control

**File:** `handlers/status_led_manager.h`

Controls WS2812B RGB LED with system state feedback.

**Methods:**

- `begin()` - Initialize NeoPixel
- `run(now, bleState, batteryPercent)` - Update LED based on state
- `setBootState(state)` - Indicate boot progress

**LED States:**

- **Pairing**: Blinking blue @ 250ms
- **Connected**: Solid blue
- **Low Battery** (≤20%): Blinking red @ 500ms
- **Normal**: Gradient color based on battery percentage
    - ≥60%: Yellow to green
    - 21-59%: Red to green

### 7. **DisplayManager** - OLED Rendering

**File:** `handlers/display_manager.h`

Manages SSD1306 OLED display with multiple screen views.

**Methods:**

- `begin()` - Initialize display
- `run()` - (Updates triggered by SystemOrganizer)
- `update()` - Redraw current screen
- `nextScreen()` / `previousScreen()` - Screen navigation
- `getCurrentScreen()` - Query active screen
- `clear()` - Turn off display
- `showBootFlash()` - Show boot screen

**Screens:** 0. **Matrix**: Shows last pressed key

1. **Battery**: Shows percentage & voltage
2. **Inputs**: Shows button states (BT, VBUS, ENC)
3. **Debug**: Shows scrolling log buffer

### 8. **SystemOrganizer** - Main Coordinator

**File:** `system_organizer.h`

Central orchestrator that manages initialization and runtime loop.

**Methods:**

- `begin()` - Initialize all subsystems in order
- `run()` - Main loop with state machine
- `goToSleep()` - Enter deep sleep mode

**Initialization State Machine:**

1. `kDelayBeforeInit` - Wait 1 second
2. `kDelayBeforeOled` - Configure I2C & I/O, wait 100ms
3. `kShowingBootFlash` - Display boot message for 2 seconds
4. `kRunning` - Normal operation

**Runtime Loop:**

- Matrix scanning every 2ms
- Encoder processing on interrupt
- Input state refresh every 150ms or on change
- Display update on state change
- LED update every 50ms
- Sleep detection (2-second encoder hold)

## Data Flow

### Initialization Flow

```
setup() → SystemOrganizer::begin()
  ├─ Enable LDO2 power
  ├─ Initialize logger (Serial)
  ├─ Initialize I2C bus
  ├─ Initialize all handlers
  └─ Set encoder ISR static instance
```

### Runtime Flow (Normal Operation)

```
loop() → SystemOrganizer::run()
  ├─ [If not ready] Run initialization state machine
  └─ [If ready] runNormal():
      ├─ Check power button (encoder hold)
      ├─ Scan matrix keypad periodically
      ├─ Process encoder if interrupt
      ├─ Read digital inputs periodically
      ├─ Update battery percentage on change
      ├─ Render display on state change
      └─ Update LED color
```

## Benefits of Modularization

1. **Separation of Concerns**: Each handler has a single responsibility
2. **Reusability**: Handlers can be used in other projects or tests
3. **Testability**: Individual components can be tested in isolation
4. **Maintainability**: Changes to one component don't affect others
5. **Scalability**: Easy to add new handlers without cluttering main.cpp
6. **Clear Dependencies**: Dependencies explicitly shown in constructors
7. **Thin Entry Point**: main.cpp now just creates app instance and calls begin()/run()

## File Structure

```
firmware-test/
├── platformio.ini
├── include/
│   ├── config.h                    ← Central configuration
│   ├── system_organizer.h          ← Main coordinator
│   └── handlers/
│       ├── debug_logger.h          ← Logging system
│       ├── battery_manager.h       ← Fuel gauge reader
│       ├── status_led_manager.h    ← RGB LED control
│       ├── matrix_handler.h        ← Keypad scanner
│       ├── input_manager.h         ← Digital inputs & encoder
│       └── display_manager.h       ← OLED display rendering
├── src/
│   ├── main.cpp                    ← Thin entry point
│   └── input_manager.cpp           ← Static member definition
├── lib/
└── test/
```

## Key Design Patterns Used

### Dependency Injection

Handlers receive dependencies through constructors:

```cpp
DisplayManager(DebugLogger& logger, MatrixHandler& matrix,
               BatteryManager& battery, InputManager& inputs)
```

### State Machine

SystemOrganizer manages initialization via enum-based state machine preventing race conditions during startup.

### Circular Buffer

DebugLogger uses a simple circular buffer for log display, automatically scrolling old messages.

### Interrupt Handler Pattern

InputManager uses a static pointer to maintain interrupt handler association with instance methods.

### Encapsulation

All state is private with minimal public interface. Only methods needed by SystemOrganizer are exposed.

## Extending the System

To add a new subsystem:

1. Create a new handler header in `include/handlers/NewHandler.h`
2. Implement `begin()` and `run()` methods
3. Add to SystemOrganizer as a member variable
4. Initialize in `SystemOrganizer::begin()`
5. Call in `SystemOrganizer::runNormal()` or appropriate place

**Example:**

```cpp
class WifiHandler {
 public:
  WifiHandler(DebugLogger& logger) : logger_(logger) {}

  void begin() {
    logger_.log("WIFI: Initializing");
    // WiFi setup code
  }

  void run() {
    // Periodic WiFi tasks
  }

 private:
  DebugLogger& logger_;
};
```

## Migration Notes

- All global variables are now encapsulated in handler classes
- Pin constants moved to `config.h`
- Timing constants moved to `config.h`
- Original functionality preserved exactly
- Interrupt handlers now properly managed through static instance
