# PROS3 Macropad — Development TODO

---

## Phase 1: Firmware

> **Guiding principles:** power efficiency, clean HID/serial communication, and a lightweight core that's easy to extend later.

### 1.1 Project Setup ✅

- [x] Initialize PlatformIO project targeting `esp32-s3-devkitc-1` (or the UM ProS3 board definition)
- [x] Configure `platformio.ini` — set framework, partition scheme, serial monitor baud rate, and build flags
- [x] Establish a clean folder structure (`src/`, `include/`, `lib/`) with a minimal `main.cpp` that boots and blinks an LED

### 1.2 Debug & Logging ✅

- [x] Create a `debug.h` module with leveled log macros: `DBG_ERROR`, `DBG_WARN`, `DBG_INFO`, `DBG_VERBOSE`
- [x] Guard all debug output behind a compile-time flag (`-DDEBUG_LEVEL=n` in `platformio.ini`) — zero overhead in release builds
- [x] Route debug output to USB CDC serial (separate from the listener data channel, or clearly prefixed e.g. `#DBG: ...`)
- [x] Log key subsystem events: boot sequence, config load/parse result, layer switch, sleep/wake transitions, HID report sends, BLE connect/disconnect
- [x] Add a timestamp (millis) to every debug line for easy profiling
- [x] (Optional) Add a runtime toggle via a serial command (e.g., `DBG:ON` / `DBG:OFF`) to enable verbose logging without reflashing

### 1.3 Power Management ✅

- [x] Configure ESP32-S3 clock speed and power domains — disable unused peripherals (WiFi radio, unused GPIOs) at boot
- [x] Implement light-sleep mode that activates after a configurable idle timeout (no key/encoder input)
- [x] Wake from light-sleep on any key matrix GPIO change or encoder interrupt
- [x] Gate the SK6812 LED power rail via the TPS22918 load switch — turn LEDs off during sleep
- [x] Gate or dim the OLED during idle (SSD1306 display-off command) before entering sleep
- [x] Expose a simple `power_config` struct so thresholds/timeouts can be tuned in one place

### 1.4 Input Scanning ✅

- [x] Implement the 3×4 key matrix scan with proper debouncing (≤5 ms latency target)
- [x] Use interrupts rather than polling where possible to stay idle between events
- [x] Read rotary encoder via hardware interrupt — track direction and press/click events
- [x] Implement a lightweight event queue (ring buffer) that other subsystems can consume

### 1.5 Layer System ✅

- [x] Implement a 4-layer state machine driven by the encoder button
- [x] Layer 0 (FN Keys): hardcoded F13–F24 HID keycodes
- [x] Layers 1–3: read mappings from a `config.json` stored in LittleFS/SPIFFS
- [x] Add safe fallback — if config is missing or corrupt, default to Layer 0 behavior on all layers

### 1.6 USB HID

- [ ] Enable the ESP32-S3 native USB stack (TinyUSB) as a composite device: **Keyboard + CDC Serial**
- [ ] Implement HID keyboard reports — send single keys and modifier combos (`Ctrl+Shift+P`, etc.)
- [ ] Implement HID consumer control reports for media keys (volume up/down via encoder on Layer 0/1)
- [ ] Make sure HID descriptors are minimal and spec-compliant to avoid driver issues across OSes

### 1.7 BLE HID

- [ ] Integrate NimBLE (lightweight) for Bluetooth Low Energy HID
- [ ] Implement BLE HID keyboard + consumer control services mirroring the USB HID feature set
- [ ] Add secure pairing (just-works or passkey via OLED display)
- [ ] Implement a hardware or software toggle (slide switch / long-press) to switch between USB and BLE modes
- [ ] When in BLE mode, disable the USB HID interface cleanly (and vice-versa) to avoid conflicts

### 1.8 Serial Communication (Host Listener Protocol)

The old `L2:C0R1` format (9 bytes + newline) is replaced with a **compact single-byte key ID** scheme:

| Byte bits | Meaning                                                                         |
| --------- | ------------------------------------------------------------------------------- |
| `[7:6]`   | Layer (0–3)                                                                     |
| `[5:4]`   | Column (0–2)                                                                    |
| `[3:2]`   | Row (0–3)                                                                       |
| `[1:0]`   | Event type: `00` = press, `01` = release, `10` = encoder CW, `11` = encoder CCW |

This encodes every possible event in **1 byte** (sent as 2 hex chars + `\n` = 3 bytes total, vs 10 before). Example: Layer 2, Col 1, Row 3, press → `0b10_01_11_00` → `0x9C` → send `9C\n`.

- [x] Define the compact protocol in a shared header (`protocol.h`) with encode/decode helpers
- [ ] On Layers 2 & 3, send the encoded byte over **USB CDC serial**
- [ ] On Layers 2 & 3, send the same encoded byte over **BLE Serial (Nordic UART Service)** when in BLE mode
- [x] Prefix debug/log lines with `#` so the listener can trivially separate data from debug output on the same channel
- [ ] (Optional) Add a lightweight handshake / heartbeat so the host listener can detect connection state
- [ ] Ensure serial output does not block — use a small TX buffer and drop if full

### 1.9 OLED Display

- [ ] Initialize SSD1306 128×32 via I²C using U8g2
- [ ] Display: current layer name, connection mode (USB / BLE), and a status icon area
- [ ] Keep draw calls efficient — only redraw on state change, not every loop iteration
- [ ] Add a simple screen-off timeout that ties into the power management idle timer

### 1.10 RGB Underglow

- [ ] Initialize 10× SK6812MINI via FastLED on the correct data pin
- [ ] Implement a few built-in effects: solid color, breathing, rainbow cycle
- [ ] Accept color, brightness, speed, and mode from the `config.json` lighting section
- [ ] Cap max brightness in firmware to protect current draw, especially on battery/BLE mode

### 1.11 Config Loading (via USB from Host Listener)

- [ ] Mount LittleFS/SPIFFS partition at boot
- [ ] Implement a USB serial command to receive a new `config.json` from the host listener (e.g., a `CFG:` prefixed transfer or a simple chunked write protocol)
- [ ] Write the received config to the LittleFS/SPIFFS filesystem and acknowledge success/failure back over serial
- [ ] Parse the stored `config.json` at boot using ArduinoJson
- [ ] Validate the structure on load — reject gracefully and fall back to defaults on error
- [ ] Trigger a live reload of layer mappings and lighting settings when a new config is received (no reboot required)

---

## Phase 2: Host Listener

> **Guiding principles:** fast & reliable serial communication, good UX for the background service. Everything else comes later.

### 2.1 Core Serial Communication

- [ ] Create `host-listener/listener.py` with a clean entry point
- [ ] Use `pyserial` to auto-detect and connect to the macropad's CDC serial port (by VID/PID or device name)
- [ ] Implement automatic reconnection — if the USB cable is unplugged, keep scanning and reconnect silently
- [ ] Decode incoming compact protocol bytes (2 hex chars + newline) into layer, column, row, and event — dispatch to an action executor
- [ ] Ignore lines starting with `#` (firmware debug output) — optionally forward them to a debug log file
- [ ] Load `config.json` at startup and watch the file for changes — hot-reload without restarting

### 2.2 Config Upload to Board

- [ ] Implement a command to push the current `config.json` to the macropad over USB serial (matching the firmware's receive protocol)
- [ ] Show upload progress and confirm success/failure from the board's acknowledgement
- [ ] Auto-push config on file change (or provide a manual trigger in the tray menu)

### 2.3 Action Execution

- [ ] Implement action types: `open_url`, `run_command`, `launch_app`
- [ ] Run actions asynchronously (subprocess / threading) so the listener never blocks on a slow command
- [ ] Add basic logging (to file and stdout) — log every received command and its result for debugging

### 2.4 User Experience

- [ ] Add a minimal system-tray icon (using `pystray` or similar) with status indicator (connected / disconnected)
- [ ] Show a native OS notification on first connect and on errors
- [ ] Add a "Browse for config…" option in the tray menu for first-time setup
- [ ] Package with `pyinstaller` or similar so end-users don't need a Python install

---

_Anything beyond these items (advanced macros, on-device config editing, Bluetooth config transfer, etc.) will be added once the foundation above is solid._
