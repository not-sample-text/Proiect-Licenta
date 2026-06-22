# Code Migration Guide: firmware-test → firmware-prod

This folder contains production-ready components extracted from firmware-test.

## Components Included

### 1. **Matrix Scanning + Debounce**

- **File**: `matrix_handler.h`
- **Features**:
    - 4x3 keypad matrix scanning
    - 5ms debounce on key presses
    - Tracks last pressed key and stable state per key
    - Logs key presses to debug logger

### 2. **Status LED Logic**

- **File**: `status_led_manager.h`
- **Features**:
    - WS2812B RGB LED control (1x LED, pin 18)
    - Battery level color gradient (green→yellow→red)
    - BLE state indication (pairing=blue blink, connected=blue, low battery=red blink)
    - Boot state gating to prevent early LED updates

### 3. **OLED Display Manager**

- **File**: `display_manager.h`
- **Features**:
    - 128x32 SSD1306 OLED (I2C: pins 8/9)
    - 5 screen rotation:
        - Matrix: shows last pressed key
        - Battery: percentage + voltage
        - Inputs: BT button, VBUS, encoder switch states
        - Debug: scrolling log buffer
        - **NEW: Encoder Debug**: raw CLK/DT pin states + accumulated steps

### 4. **Battery Management**

- **File**: `battery_manager.h`
- **Features**:
    - MAX17048 fuel gauge (I2C address 0x36)
    - Battery percentage (0-100%)
    - Voltage in millivolts
    - Error handling for I2C failures

### 5. **Input Manager + Encoder Debug** ⭐

- **Files**: `input_manager.h`, `input_manager.cpp`
- **Features**:
    - VBUS sense (USB power detection)
    - Bluetooth button (pin 34, active-low)
    - Encoder switch (pin 1, active-low)
    - **Quadrature encoder with state machine** (pins 2=CLK, 4=DT)
    - **NEW DEBUG ACCESSORS**:
        - `getEncoderAccumulatedSteps()` - returns -3 to +3
        - `getEncoderLastState()` - returns 0-3 state code (bits: 1=CLK, 0=DT)
    - **NEW DEBUG LOGGING**: Set `kEncoderDebugLogging = true` in config.h

## Rotary Encoder Issue: Diagnosis

**Problem**: OLED screens switch every 2 detents instead of every full rotation

**Root Cause**: Likely one of these:

1. **Detent alignment**: Encoder detents not matching state transitions
2. **Transition threshold**: Current code returns step at `±4` accumulated, but encoder may produce fewer transitions per detent
3. **Screen transition logic**: SystemOrganizer may be calling `nextScreen()` on partial steps

**How to Debug**:

1. Enable encoder debug in `config.h`:

    ```cpp
    constexpr bool kEncoderDebugLogging = true;
    ```

2. Rotate encoder slowly and watch the debug log or serial output for:
    - State transitions (S0→S1→S3→S2→S0 = 1 CW detent)
    - Accumulated step counter
    - When "ENC: CW Step" is logged

3. Use the new **Encoder Debug Screen** (press encoder to rotate through screens):
    - Shows real-time CLK/DT pin states
    - Shows accumulated steps counter
    - Shows interrupt pending flag

**Expected Behavior**:

- 1 full CW rotation = 4 state transitions = 1 step output
- Each detent (audible click) should be ¼ rotation, so 2 detents = 1 step output

## Integration Steps

### Step 1: Copy handler headers

Copy these to `firmware-prod/include/handlers/`:

- `matrix_handler.h`
- `status_led_manager.h`
- `display_manager.h`
- `battery_manager.h`
- `debug_logger.h`
- `input_manager.h`

### Step 2: Copy implementation file

Copy to `firmware-prod/src/`:

- `input_manager.cpp`

### Step 3: Update config

Copy `config.h` to `firmware-prod/include/`, then merge your prod-specific settings:

- Encoder pins (already set: CLK=2, DT=4)
- Other board-specific pins
- Timing parameters

### Step 4: Update SystemOrganizer

Create/modify `firmware-prod/include/system_organizer.h` to:

1. Instantiate all handlers
2. Call `begin()` on each during setup
3. Call `run()` on each in the main loop
4. Route encoder events to `display_.nextScreen()` / `display_.previousScreen()`

### Step 5: Test encoder

1. Check serial output for state transitions
2. Rotate encoder slowly and count state logs
3. Verify screen transitions align with full detents (not every 2)

## Configuration

Edit `config.h` to adjust:

- **Encoder**: `kEncoderDebugLogging` (boolean flag)
- **Matrix**: `kMatrixDebounceMs` (currently 5ms)
- **Status LED**: `kStatusLedBlinkMs` (currently 250ms)
- **Battery**: Fuel gauge address (0x36, standard)

## Dependencies

- Arduino framework
- Adafruit_NeoPixel (for RGB LED)
- Adafruit_SSD1306 (for OLED)
- Adafruit_GFX (for OLED graphics)
- Wire/I2C (built-in)

## Notes

- All handlers are header-only except `input_manager.cpp`
- ISR handler (`inputManagerEncoderISR`) must remain in a separate .cpp file for ESP32 IRAM safety
- Log buffer is 4 lines × 21 characters max (check `config.h`)
- Interrupt-driven encoder for low-latency response
