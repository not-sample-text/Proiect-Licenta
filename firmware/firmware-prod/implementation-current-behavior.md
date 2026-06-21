# Firmware Current Implementation Behavior

## Scope

This document describes what the current firmware implementation does today, based on the orchestrator in [firmware/lib/MacropadApp/MacropadApp.cpp](lib/MacropadApp/MacropadApp.cpp) plus modules under [firmware/src](src), [firmware/include](include), and [firmware/lib](lib).

It intentionally describes actual behavior (not desired architecture yet).

## High-Level Runtime Model

The firmware is a polling main loop with interrupt-assisted input capture:

1. `setup()` (delegated into `MacropadApp::begin()`) initializes serial, pins, power, config, layers, keymap, matrix scanner, encoder, OLED, RGB, then transport mode (BLE/USB).
2. `loop()` (delegated into `MacropadApp::run()`) continuously scans matrix + encoder, drains the global input queue, processes serial commands, runs power checks, updates OLED/RGB, and services HID stack.

Core orchestrator: [firmware/lib/MacropadApp/MacropadApp.cpp](lib/MacropadApp/MacropadApp.cpp)

## Boot Sequence (`setup`)

Defined in [firmware/lib/MacropadApp/MacropadApp.cpp](lib/MacropadApp/MacropadApp.cpp):

1. Initializes USB CDC serial (`Serial.begin(115200)`), waits up to 2s for CDC enumeration.
2. Logs boot banner using debug macros.
3. Configures all board pins in `init_pins()`.
4. Initializes power manager (`power_init`).
5. Mounts and loads config (`config_init`) from LittleFS.
6. Initializes layer state (`layers_init`) and keymap resolver (`keymap_init`).
7. Initializes matrix scanner (`matrix_init`) and encoder (`encoder_init`).
8. Initializes display (`oled_init`) and RGB (`rgb_init`).
9. Reads BT/USB mode switch (`ble_is_enabled`):
    - BLE mode: initializes BLE HID stack (`ble_hid_init`) and sets OLED mode text to BLE.
    - USB mode: sets OLED mode text to USB.
10. Sets initial OLED layer and encoder mode labels.
11. Blinks status LED 3 times.
12. Initializes USB HID (`setup_usb_hid`).

Observations:

- USB HID setup runs regardless of BLE mode selection.
- BLE mode is sampled once at boot and stored in `g_ble_mode`.

## Main Loop Behavior (`loop`)

Order in [firmware/lib/MacropadApp/MacropadApp.cpp](lib/MacropadApp/MacropadApp.cpp):

1. `matrix_scan()`
2. `encoder_process()`
3. `process_input_events()`
4. `process_serial_commands()`
5. `check_power_idle()` (throttled to once per 1s)
6. `oled_update()`
7. `rgb_update()`
8. Transport task:
    - BLE mode: `ble_hid_task()`
    - USB mode: `usb_hid_task()`
9. `delay(1)`

Practical effect: key/encoder events are generated first, consumed immediately in the same iteration if queue is not backlogged.

## Global Data and Coordination

### Runtime globals in main

In [firmware/src/main.cpp](src/main.cpp):

- `g_debug_enabled`: gates debug output at runtime.
- `g_ble_mode`: chooses BLE vs USB dispatch path.

### Shared event pipeline

- Producer modules: matrix scanner and encoder.
- Queue: global `g_input_queue` in [firmware/src/input_events.cpp](src/input_events.cpp).
- Consumer: `process_input_events()` in [firmware/src/main.cpp](src/main.cpp).

## Module-by-Module Current Behavior

### 1) Debug Logging

Files: [firmware/include/debug.h](include/debug.h)

- Macros output lines in format `#[millis][TAG] message`.
- Compile-time filtering by `DEBUG_LEVEL` macro.
- Runtime filtering by `g_debug_enabled`.
- Output always uses `Serial.printf`.

### 2) Input Event Queue

Files: [firmware/include/input_events.h](include/input_events.h), [firmware/src/input_events.cpp](src/input_events.cpp)

- Ring buffer with capacity 32.
- SPSC-style logic (one producer context, one consumer context assumed).
- `enqueue` drops when full.
- Event includes type, col, row, timestamp.

### 3) Matrix Scanner (3x4)

Files: [firmware/include/matrix.h](include/matrix.h), [firmware/src/matrix.cpp](src/matrix.cpp)

- Columns driven active-low one by one.
- Rows read with external pull-ups.
- Debounce per key using timestamped raw state stabilization.
- Generates `EVENT_KEY_PRESS` and `EVENT_KEY_RELEASE` into queue.
- Calls `power_activity()` when a debounced key transition is accepted.
- Attaches row interrupts (`CHANGE`) to set `g_scan_needed` flag.

Important nuance:

- `g_scan_needed` is set by ISR but currently not used as a gate in `matrix_scan`; scanning still runs every loop.

### 4) Rotary Encoder

Files: [firmware/include/encoder.h](include/encoder.h), [firmware/src/encoder.cpp](src/encoder.cpp)

- Rotation decoding is ISR-based via CLK and DT interrupts.
- Uses 4-bit transition table (`ENCODER_TABLE`) for quadrature direction.
- Accumulates sub-steps to detent threshold (`detent_steps`, default 4).
- On full detent:
    - increments/decrements `g_encoder_position`
    - enqueues `EVENT_ENCODER_CW` or `EVENT_ENCODER_CCW`
    - calls `power_activity()`
- Button processing is polled in `encoder_process()` with debounce.
- Encoder mode tracked globally:
    - `ENCODER_MODE_VOLUME` (default)
    - `ENCODER_MODE_LAYER`

### 5) Layer State Machine

Files: [firmware/include/layers.h](include/layers.h), [firmware/src/layers.cpp](src/layers.cpp)

- 4 layers:
    - Layer 0: FN Keys
    - Layer 1: User
    - Layer 2: Serial A
    - Layer 3: Serial B
- Supports set, next, previous cycle.
- Optional single callback on changes.
- Default layer is config struct default (Layer 0).

### 6) Keymap Resolution

Files: [firmware/include/keymap.h](include/keymap.h), [firmware/src/keymap.cpp](src/keymap.cpp)

Resolution rules in `keymap_get_action`:

- Layer 0: always returns hardcoded F13..F24 mapping by key position.
- Layers 2/3: always returns `ACTION_SERIAL_SEND`.
- Layer 1: tries `config_get_key_action`, parses action string; fallback to Layer 0 F13..F24 mapping if missing/invalid.

Supported parsed action formats:

- `F13`..`F24`
- `A`..`Z`
- Modifier combos like `Ctrl+Shift+C`
- Media prefix: `Media:VolUp`, `Media:VolDown`, `Media:Mute`, `Media:Play`, `Media:Next`, `Media:Prev` (plus aliases)

### 7) Config Storage and Parsing

Files: [firmware/include/config.h](include/config.h), [firmware/src/config.cpp](src/config.cpp)

- Uses LittleFS and ArduinoJson.
- Config path: `/config.json`.
- `config_init()` mounts FS then attempts parse.
- `config_load()`:
    - validates file exists and size bounds
    - deserializes JSON into global document
    - extracts RGB fields (`brightness`, `mode`, `speed`, `color`)
- `config_save()` validates JSON then writes and reloads.
- Accessors:
    - `config_get_key_action(layer,col,row)`
    - `config_get_key_label(layer,col,row)`
    - RGB and power timeout getters

Key detail:

- Parsing of `rgb.mode` checks `is<const char*>()` but assigns to numeric field (`uint8_t`), so effective behavior depends on input format and implicit conversions.

### 8) USB HID Path

Files: [firmware/include/usb_hid.h](include/usb_hid.h), [firmware/src/usb_hid.cpp](src/usb_hid.cpp)

- Uses TinyUSB APIs.
- Sends keyboard reports via `tud_hid_keyboard_report`.
- Sends consumer reports via `tud_hid_report(report_id=1, ...)`.
- `usb_hid_task()` runs `tud_task()` each loop (when USB mode selected).
- Volume helper functions send press then release with short delays.

### 9) BLE HID Path

Files: [firmware/include/ble_hid.h](include/ble_hid.h), [firmware/src/ble_hid.cpp](src/ble_hid.cpp)

- Uses NimBLE stack.
- Creates server and HID device, sets report descriptor for keyboard+consumer.
- Exposes keyboard input report ID 1 and consumer report ID 2.
- Starts advertising after setup and after disconnect callbacks.
- `ble_send_key` builds 8-byte keyboard report and notifies characteristic.
- `ble_send_consumer` sends 16-bit usage report then sends release (0) after delay.

State model:

- `BLE_DISCONNECTED`, `BLE_ADVERTISING`, `BLE_CONNECTED`, `BLE_PAIRED` enum exists.
- Runtime transitions currently use disconnected/advertising/connected.

### 10) OLED Display

Files: [firmware/include/oled.h](include/oled.h), [firmware/src/oled.cpp](src/oled.cpp)

- Driver: U8g2 SSD1306 128x32 over hardware I2C.
- State-driven redraw with dirty flag `g_needs_redraw`.
- Displays:
    - top-left: layer name
    - top-right: battery percentage + icon
    - BLE icon when in BLE mode
    - bottom-left: encoder mode
    - bottom-right: last key label (truncated when too long)
- Supports temporary status message overlay with timeout.
- Supports sleep/wake/dim/bright operations.

Startup behavior:

- Shows splash screen then clears after 1s delay.

### 11) RGB Underglow

Files: [firmware/include/rgb.h](include/rgb.h), [firmware/src/rgb.cpp](src/rgb.cpp)

- Driver: FastLED with SK6812 template.
- LED count from pin config.
- Modes:
    - OFF
    - SOLID
    - BREATHING
    - RAINBOW
    - CYCLE
- Pulls initial settings from config RGB block.
- Brightness capped at `RGB_MAX_BRIGHTNESS`.
- Update interval derived from `g_rgb_speed` (roughly 10..100ms).

### 12) Power Management

Files: [firmware/include/power.h](include/power.h), [firmware/src/power.cpp](src/power.cpp)

- Tracks `g_last_activity_ms` for idle checks.
- `power_activity()` refreshes last activity and reverts to active mode from light sleep.
- Modes:
    - ACTIVE (max 240MHz)
    - LIGHT_SLEEP (80..10MHz + light sleep enabled)
    - DEEP_SLEEP (reboot on wake)
- Idle policy in `power_check_idle()`:
    - > 10s: enter light sleep mode config
    - > 30s: enter deep sleep
- Wake source configured as encoder button pin (ext0 low).

Battery reporting:

- ADC reads VBAT with averaging.
- Converts ADC to mV assuming 2:1 divider and 3.3V reference.
- Percent uses simple linear mapping 3.0V..4.2V.
- Charging derived from USB present + battery voltage < 4.15V.

## End-to-End Input/Event Dispatch Flow

1. Physical key/encoder changes are detected by matrix scanner / encoder ISR.
2. Events are enqueued into `g_input_queue`.
3. Main loop drains queue in `process_input_events()`.
4. Dispatch behavior by event type:
    - Encoder button press: toggles encoder mode and updates OLED.
    - Encoder CW/CCW:
        - Volume mode: sends volume HID command (BLE or USB path).
        - Layer mode: cycles layers and updates OLED.
    - Key press/release:
        - Optionally updates OLED label on press.
        - Resolves action from keymap.
        - Action dispatch:
            - HID key: send key press/release (BLE or USB)
            - HID media: send on press only
            - Serial action: emits packed protocol byte to serial

Protocol encoding helper: [firmware/include/protocol.h](include/protocol.h)

## Serial Command Channel (Host Interaction)

Implemented in [firmware/src/main.cpp](src/main.cpp), function `process_serial_commands`.

Supported commands:

- `DBG:ON`
- `DBG:OFF`
- `CFG:START:<size>`
- `CFG:DATA:<chunk>`
- `CFG:END`
- `CFG:ABORT`

Responses:

- prefixed status responses such as `#CFG:READY`, `#CFG:ACK`, `#CFG:SUCCESS`, error lines.

Upload behavior:

- Receives plain command text, appends chunk strings to `g_cfg_buffer`, checks final exact size match, then persists via `config_save`.
- On successful save/reload, reapplies RGB parameters and posts OLED status message.

## Current Behavioral Edge Cases / Notes

These are current-behavior notes, not yet change proposals:

1. BLE/USB mode is effectively static after boot (`g_ble_mode` not re-sampled later).
2. Matrix ISR wake flag (`g_scan_needed`) is currently informational; matrix scan still runs every loop.
3. Several paths rely on short delays (e.g., HID consumer release timing, OLED splash, serial wait).
4. Input queue overflow silently drops ISR-generated events (except some non-ISR code paths log warnings).
5. Config action parsing supports a limited key vocabulary (A-Z + F13-F24 + listed media names).
6. Layer 1 missing mappings fall back to Layer 0 F-key defaults.
7. `oled.cpp` and serial command parser use Arduino `String` extensively for transient/state text.

## Practical “What It Does” Summary

Current firmware behaves as a configurable 12-key + encoder macropad with:

- Dual output mode selected at boot (BLE HID or USB HID)
- 4 logical layers (FN/User/SerialA/SerialB)
- Encoder with two user-facing modes (volume or layer cycle)
- OLED UI for active context and key feedback
- RGB effects with config-controlled parameters
- LittleFS-based runtime config upload via serial commands
- Idle power transitions to light sleep and deep sleep

This is the baseline implementation to compare against any upcoming architecture/code changes.
