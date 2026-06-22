# to-migrate/ — Code for Production Migration

This folder contains hardware handler implementations extracted from **firmware-test** for integration into **firmware-prod**.

## What's Here

| File                   | Component                                | Status       |
| ---------------------- | ---------------------------------------- | ------------ |
| `config.h`             | Hardware pins & timing constants         | ✅ Ready     |
| `debug_logger.h`       | Serial + OLED log buffer                 | ✅ Ready     |
| `matrix_handler.h`     | 4×3 keypad + debounce                    | ✅ Ready     |
| `status_led_manager.h` | WS2812B RGB LED control                  | ✅ Ready     |
| `battery_manager.h`    | MAX17048 fuel gauge reader               | ✅ Ready     |
| `display_manager.h`    | OLED (+ **NEW encoder debug screen**)    | ✅ Enhanced  |
| `input_manager.h`      | Buttons + **enhanced encoder debugging** | ✅ Enhanced  |
| `input_manager.cpp`    | ISR implementation (IRAM-safe)           | ✅ Ready     |
| `MIGRATION_GUIDE.md`   | Integration steps & encoder debugging    | 📖 Full docs |

## Quick Start

1. **Read** `MIGRATION_GUIDE.md` for full integration steps
2. **Copy** all `.h` and `.cpp` files to firmware-prod
3. **Merge** `config.h` with your prod-specific settings
4. **Update** SystemOrganizer to instantiate and manage handlers
5. **Test** encoder using new debug screen (5th screen in rotation)

## Encoder Debugging ⭐

A new **Encoder Debug Screen** has been added to help diagnose the "switches every 2 detents" issue:

- **Access**: Press encoder to cycle through 5 screens (new: screen 5)
- **Shows**:
    - Raw CLK/DT pin states (0 or 1)
    - Accumulated step counter (-4 to +4)
    - Last state code (0-3)
    - Interrupt pending flag

Enable detailed logging in `config.h`:

```cpp
constexpr bool kEncoderDebugLogging = true;
```

This logs every state transition and step to the debug logger and serial output.

## Key Improvements Made

1. **Debug accessors** in InputManager:
    - `getEncoderAccumulatedSteps()` — track partial steps
    - `getEncoderLastState()` — see quadrature state code

2. **New screen** in DisplayManager:
    - Real-time encoder state visualization
    - No dependencies on interrupt flags from main loop

3. **Enhanced logging**:
    - State transition logs
    - Conditional debug output (compile-time flag)

4. **Documentation**:
    - Root cause analysis of encoder issue
    - Debugging workflow
    - Integration checklist

## Next Steps

1. Integrate handlers into firmware-prod architecture
2. Test encoder with debug mode enabled
3. Verify screen transitions align with full detents
4. Disable debug logging once tuned (`kEncoderDebugLogging = false`)
