# Firmware Architecture Refactor Status

## What Was Changed Now

The firmware entrypoint was refactored to follow a service-style structure inspired by the reference project layout (`lib/*` modules with `begin()` and `run()`).

### New internal modules

- `lib/MacropadApp/MacropadApp.h`
- `lib/MacropadApp/MacropadApp.cpp`
- `lib/BoardSupport/BoardSupport.h`
- `lib/BoardSupport/BoardSupport.cpp`
- `lib/SerialControl/SerialControl.h`
- `lib/SerialControl/SerialControl.cpp`

### Entrypoint simplification

- `src/main.cpp` now only instantiates `MacropadApp` and forwards to:
    - `MacropadApp::begin()`
    - `MacropadApp::run()`

This mirrors the pattern from the reference workspace where `main.cpp` stays minimal and application behavior lives in dedicated modules.

## Current Architectural Split

- Hardware pin setup and startup blink: `BoardSupport`
- Runtime orchestration and event dispatch: `MacropadApp`
- Serial debug/config command channel: `SerialControl`
- Existing domain modules remain in place for now:
    - matrix, encoder, layers, keymap, config, power, OLED, RGB, BLE HID, USB HID

## Functional Behavior Impact

The refactor was designed to keep behavior equivalent while improving structure.

- Boot order remains functionally the same.
- Main loop order remains functionally the same.
- BLE/USB mode selection still happens at startup.
- Serial command behavior (`DBG:*`, `CFG:*`) remains equivalent.

## Requirements Captured For Next Implementation Pass

The following points are captured and should drive the upcoming power/board logic changes:

1. VBAT path context includes TPS22918DBVR LED power switching.
2. GPIO16 should drive TPS22918DBVR enable high/low for LED rail power.
3. On any macropad shutdown path, LED rail must be turned off.
4. The divider path is used to sense USB plug/charge state.
5. Add deep-sleep mode with no wake-up pin; device should stay off until USB is plugged.
6. Use automatic light sleep at the lowest stable CPU MHz for this workload.

## Next Refactor Targets (Suggested)

1. Replace generic `PIN_LED_STATUS` semantics with explicit LED power switch naming for TPS22918 control.
2. Move power policy decisions into a dedicated class-based power policy module (`begin()/run()`).
3. Add explicit "power-down peripherals" sequence before deep sleep, including LED rail off.
4. Add USB-presence-gated startup path for near-off mode.
5. Remove remaining blocking delays and convert to non-blocking timing where practical.
