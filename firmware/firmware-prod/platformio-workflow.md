# PlatformIO Project Guide

This guide covers the recommended folder layout, environment configuration, dependency management, and coding guidelines for PlatformIO (C/C++) projects.

Building on standard embedded practices, this architecture emphasizes deterministic builds, strict dependency locking, and strict separation of hardware initialization from application logic.

## Folder Layout

Every PlatformIO project follows this minimal structure, extending the default PlatformIO scaffolding with our mandatory documentation and tooling directories:

```text
<project-name>/
├── .github/
│   └── copilot-instructions.md        # Copilot instructions file
├── .vscode/
│   └── settings.json                  # VSCode workspace settings
├── docs/
│   └── 01_specificatii-initiale.md    # Initial project specifications (from client)
├── include/                           # Global headers
│   └── config.h                       # Global pin definitions, intervals, constants
├── lib/                               # Project-specific (internal) libraries
│   ├── BME280/                        # Custom sensor wrapper
│   └── Relay/                         # Custom relay driver
├── src/                               # Application source code
│   └── main.cpp                       # Main application entry point
├── .gitattributes                     # Git attributes file
├── .gitignore                         # Git ignore file
├── platformio.ini                     # PlatformIO configuration and environments
└── README.md                          # Project overview and documentation
```

## Configuration File (`platformio.ini`)

The `platformio.ini` file is the source of truth for the build environment.

**Absolute Rules for `platformio.ini`:**

- **ALWAYS** lock the platform version (e.g., `platform = espressif32@~6.3.0`). NEVER use `platform = espressif32` without a version, as future updates will break deterministic builds.
- **ALWAYS** lock library versions in `lib_deps` (e.g., `bblanchon/ArduinoJson @ ^6.21.3`).
- **ALWAYS** use separate environments (`[env:debug]`, `[env:release]`) if you have different build flags for development and production.

```ini
[platformio]
default_envs = release

[env]
platform = espressif32@~6.3.0
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600

[env:release]
build_flags =
    -D CORE_DEBUG_LEVEL=0
    -D RELEASE_BUILD

[env:debug]
build_flags =
    -D CORE_DEBUG_LEVEL=4
    -D DEBUG_BUILD
lib_deps =
    # Example of strict versioning for external dependencies
    bblanchon/ArduinoJson @ ^6.21.3
    adafruit/Adafruit BME280 Library @ ^2.2.2
```

## Library Management

- **External Libraries**: **MUST** be defined in `platformio.ini` under `lib_deps` with strict semantic versioning. **NEVER** install libraries globally using the PlatformIO CLI or VS Code GUI, as this breaks project portability.
- **Internal Libraries**: Custom drivers, wrappers, and modular business logic **MUST** live in the `lib/` directory. Each library must have its own folder containing its `.h` and `.cpp` files.

## Naming Conventions

### Files & Folders

- **Headers & Sources**: `PascalCase.h` and `PascalCase.cpp` for libraries (e.g., `RelayController.h`).
- **Main file**: `main.cpp` (must be lowercase).

### Code Identifiers

- **Constants**: `UPPER_SNAKE_CASE` — defined as `constexpr` in `include/config.h`.
- **Macros**: **DO NOT** use `#define` macros for constants. All constants **MUST** be declared with `constexpr` for type safety, scoping, and debugger visibility. The only acceptable use of `#define` is for include guards or conditional compilation (`#ifdef DEBUG_BUILD`).
- **Include guards**: Use `__UPPER_SNAKE_CASE_H__` with double-underscore prefix and suffix (e.g., `__CONFIG_H__`).
- **Functions**: `snake_case` (e.g., `read_sensor_data()`).
- **Classes**: `PascalCase` (e.g., `RelayController`, `NetworkManager`).
- **Public methods**: `camelCase` (e.g., `setInterval()`, `connectToWifi()`).
- **Variables**: `lower_snake_case` (e.g., `sensor_value`, `connection_retries`).
- **Private/protected variables**: `_lower_snake_case` (e.g., `_last_toggle_time`).
- **Private/protected methods**: `_camelCase` (e.g., `_updateDisplay()`).

### Hardware Definitions (`config.h`)

- **Pin definitions**: `UPPER_SNAKE_CASE` with a `PIN_` prefix (e.g., `PIN_RELAY_MAIN`). Declared as `constexpr uint8_t` in `include/config.h`.
- **Intervals**: `UPPER_SNAKE_CASE` with an `INTERVAL_` prefix and a `_MS` suffix (e.g., `INTERVAL_SENSOR_READ_MS`). Declared as `constexpr uint32_t`.

**NEVER** hardcode pin numbers or magic timing values in `src/` or `lib/` files. **ALWAYS** reference `include/config.h`.

## Class Definitions

To ensure stability and avoid undefined behavior during C++ static initialization, hardware functions (like `pinMode()`, `Serial.begin()`, or I2C setup) **MUST NOT** be called in a constructor.

Each hardware-interfacing library should implement:

1. **Constructor**: Stores configuration (pins, default intervals) into private member variables.
2. **`begin()`**: One-time hardware initialization. Called explicitly in `setup()`.
3. **`run()`** or **`tick()`**: Non-blocking periodic runtime logic. Called continuously in `loop()`.

```cpp
#ifndef __STATUS_LED_H__
#define __STATUS_LED_H__

#include <Arduino.h>

class StatusLed {
    public:
        StatusLed(uint8_t pin, uint32_t interval_ms);
        void begin();
        void run();

    private:
        uint8_t _pin;
        uint32_t _blink_interval_ms;
        uint32_t _last_toggle_ms;
        bool _state;
};

#endif
```

## Coding Guidelines

- **No blocking code**: NEVER use `delay()` in the main loop or library `run()` methods. ALWAYS use non-blocking `millis()` timing patterns.
- **Data Types**: Use fixed-width integer types (`uint8_t`, `int16_t`, `uint32_t`). NEVER use `int` or `long`, as their sizes vary depending on the architecture (e.g., AVR vs ESP32).
- **Dynamic Memory**: Avoid `new`, `malloc()`, and `String` objects where possible, especially on memory-constrained AVR targets. Prefer static allocation, `std::array`, and C-strings (`char[]`).

## README.md Structure

Every PlatformIO project **MUST** include a `README.md` file in the root directory.

```md
# <Project Name>

One or two sentences describing what the project does and its target platform.

## Hardware

| Property             | Value                            |
| -------------------- | -------------------------------- |
| **Target MCU**       | e.g., ESP32-WROOM-32, ATmega328P |
| **Framework**        | e.g., Arduino, ESP-IDF           |
| **Serial baud rate** | e.g., 115200                     |

### Components

List every external component wired to the MCU:

| Component       | Quantity | Connected to     |
| --------------- | -------- | ---------------- |
| BME280 sensor   | 1        | I2C (GPIO 21/22) |
| 5V relay module | 2        | GPIO 18, GPIO 19 |

## Environment & Build

This project uses PlatformIO. To build and upload:

1. Install the PlatformIO extension in VS Code.
2. Select the correct environment (e.g., `env:release`) from the PlatformIO status bar.
3. Click **Build** and then **Upload**.

_Note: All external dependencies and platform versions are strictly locked in `platformio.ini`. No global library installation is required._
```

## .gitignore

Every PlatformIO project MUST include a `.gitignore`. Add the following to the standard Gitignore templates:

```text
# PlatformIO build and environment files
.pio/
.pioenvs/
.piolibdeps/
*.d
*.o
*.obj
*.elf
*.bin
*.hex

# VS Code PlatformIO extension artifacts
.vscode/.ropeproject
```
