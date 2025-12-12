# PROS3 Macropad - Full Technical Documentation

This document provides a detailed, "leave-no-stone-unturned" look into the hardware, firmware, and software that make up the PROS3 Macropad project. It's intended for developers, hobbyists, and anyone looking to build, customize, or fully understand the project's engineering.

---

## 1. System Architecture

The project is a complete ecosystem composed of three distinct but interconnected parts: **Hardware**, **Firmware**, and a **Software Suite** for configuration and host interaction.

The core concept is that the macropad can act as a simple HID device (like a standard keyboard) for basic shortcuts, but can also send specialized commands to a listener application on the host computer to perform more complex tasks, like launching applications or running scripts.

```mermaid
graph TD
    subgraph "Host Computer"
        A["Web Configurator (Browser)"] -->|Generates| B["config.json"];
        C["Host Listener (Python Script)"] -->|Reads| B;
    end

    subgraph "Macropad Device"
        D["Firmware (ESP32-S3)"]
        E["Hardware (Keys, Encoder, etc.)"]
    end

    E -- "User Input" --> D;
    D -- "Standard Keystrokes (HID)" --> F["Operating System"];
    D -- "Serial Commands (e.g., L2:C1R1)" --> C;
    C -- "Executes Actions (e.g., Run Notepad)" --> F;

    style B fill:#f9f,stroke:#333,stroke-width:2px
```

### The Three Pillars

1.  **Hardware (The Physical Device):** A custom-designed two-part PCB assembly featuring 12 hot-swappable keys, a rotary encoder, an OLED screen, and RGB underglow.
2.  **Firmware (Embedded C++):** The code running on the ESP32-S3 microcontroller. It scans the hardware for input, manages the display and lighting, and communicates with the host computer over USB or Bluetooth.
3.  **Software Suite (Configuration & Control):**
    *   **Web Configurator:** A browser-based GUI that allows a user to define key mappings, create macros, and customize lighting, exporting the results as a `config.json` file.
    *   **Host Listener:** A Python script that runs on the user's computer. It listens for special commands from the firmware and uses the `config.json` to translate them into actions, like running a program or a shell script.

---

## 2. Hardware Deep Dive

The hardware is designed to be both functional and easy to assemble for hobbyists. It consists of a "top" board for user interface components and a "bottom" board for the logic and power systems.

### Bill of Materials (BOM)

#### Top Board (User Interface)

| Component Name     | Qty | Value / Specification | Footprint / Part Detail   | Purpose / Function                                                            |
| :----------------- | :-: | :-------------------- | :------------------------ | :---------------------------------------------------------------------------- |
| **Keyswitch**      | 12  | Tactile Switch        | `MX_Hotswap_1.00u`        | **User Input:** Provides momentary electrical connection for key presses.     |
| **Diode**          | 12  | 1N4148                | `D_DO-35_SOD27`           | **Matrix Protection:** Prevents "ghosting" and ensures current flows one way. |
| **Rotary Encoder** |  1  | EC11 w/ Switch        | `Encoder_EC11_MX`         | **User Input:** Continuous rotational input and momentary push-button.        |
| **OLED Display**   |  1  | SSD1306 128x32        | `OLED_128x32`             | **Display Output:** Shows layers, battery status, and connection info.        |
| **SPDT Switch**    |  1  | Slide Switch          | `SLW8645745ARAND`         | **Mode Selection:** Hard switch for On/Off or Mode toggling.                  |
| **Pin Header**     |  1  | 1x15 Male Header      | `PinHeader_1x15_Vertical` | **Connection:** Connects the Top Board to the Bottom Board.                   |

#### Bottom Board (Logic & Power)

| Component Name        | Qty | Value / Specification | Footprint / Part Detail   | Purpose / Function                                                              |
| :-------------------- | :-: | :-------------------- | :------------------------ | :------------------------------------------------------------------------------ |
| **Pin Socket**        |  1  | 1x15 Female Socket    | `PinSocket_1x15_Vertical` | **Connection:** Mates with the Top Board for a modular assembly.                |
| **Microcontroller**   |  1  | UM ESP32-S3 Pro       | `DU1 / ProS3_TH`          | **Main Processor:** Firmware execution, I/O, and WiFi/BLE radio.                |
| **Antenna**           |  1  | 2.4 GHz Flexible      | `AC10200-100` / U.FL      | **Wireless:** Transmits and receives Bluetooth/WiFi signals.                    |
| **RGB LED**           | 10  | SK6812MINI            | `PLCC4_3.5x3.5mm`         | **Visuals:** Addressable, full-color underglow lighting.                        |
| **Pull-Up Resistor**  | 10  | 10k $\Omega$          | `R_0805_HandSolder`       | **Stabilization:** Ensures input pins stay at a high logic state when inactive. |
| **Limiting Resistor** | 10  | 470 $\Omega$          | `R_0805_HandSolder`       | **Protection:** Limits current flow to LEDs or data lines.                      |
| **Capacitor**         |  1  | 100nF                 | `C_0805_HandSolder`       | **Decoupling:** Filters electrical noise near the power pins.                   |

### Schematics & Layout

The full KiCad design files are located in the [`hardware/pcb/`](../hardware/pcb/) directory. For quick reference, PDF exports of the schematics and layouts are also available:

*   **Top Board:**
    *   [Schematic (`schematic-top.pdf`)](schematics/schematic-top.pdf)
    *   [Layout (`layout-top.pdf`)](schematics/layout-top.pdf)
*   **Bottom Board:**
    *   [Schematic (`schematic-bottom.pdf`)](schematics/schematic-bottom.pdf)
    *   [Layout (`layout-bottom.pdf`)](schematics/layout-bottom.pdf)

---

## 3. Firmware Deep Dive

The firmware is built using C++ in the PlatformIO environment, which provides a robust and extensible toolchain for the ESP32-S3.

### Core Functionality

-   **Input Handling:** Scans the 3x4 key matrix and the rotary encoder for events (press, release, turn, click).
-   **Layer Management:** Manages the currently active keymap layer. The rotary encoder click cycles through the 4 layers.
-   **Display Management:** Uses the U8g2 library to draw information on the OLED screen, such as the current layer name and status messages.
-   **Lighting Management:** Controls the 10 addressable RGB LEDs using the FastLED library.
-   **Communication:**
    -   **USB HID:** For Layers 0 and 1, it acts as a standard USB keyboard, sending keystrokes and media key commands (like Volume Up/Down) that are understood by any modern OS without drivers.
    -   **USB Serial:** For Layers 2 and 3, it sends a simple string command over the serial port (e.g., `L2:C0R1`) for the Host Listener to interpret. It also uses the serial port for debugging output.

### The Layer System

The firmware implements 4 distinct function layers. The active layer determines what action is performed when a key is pressed.

-   **Layer 0 (FN Keys):** Hardcoded in the firmware. This layer sends function keys F13 through F24, which are non-standard keys often used for application-specific shortcuts to avoid conflicts with standard system keys.
-   **Layer 1 (Shortcuts):** This layer is designed to send complex HID shortcuts (e.g., `Ctrl+Shift+P`). The firmware parses the `config.json` to send the correct combination of modifier keys and standard keys. *(Note: Full parsing is in development).*
-   **Layer 2 (Commands):** When a key is pressed on this layer, the firmware sends a unique identifier string for that key over the USB serial port (e.g., `L2:C0R0`). It does **not** send a keyboard command. The Host Listener must be running to receive this and execute the associated command.
-   **Layer 3 (Launcher):** Identical in mechanism to Layer 2, but uses a different prefix (e.g., `L3:C0R0`). This allows the user to organize their actions semantically (e.g., using Layer 2 for scripts and Layer 3 for launching applications).

---

## 4. Software Suite Deep Dive

### Web Configurator

The configurator, located in [`web-configurator/`](../web-configurator/), is a static HTML, CSS, and JavaScript application that can be run directly from the filesystem or a simple web server.

-   **Purpose:** To provide a user-friendly, code-free way to define the behavior of the macropad.
-   **Features:**
    -   A visual representation of the macropad layout.
    -   Configuration of key labels and actions for each of the 4 layers.
    -   Customization of RGB lighting effects, color, and brightness.
    -   Import/Export of the configuration as a single `config.json` file.
    -   Settings are automatically saved in the browser's LocalStorage.

### Host Listener

This is the critical component that unlocks the full power of the macropad. The script, `host-listener/listener.py`, runs in the background on the host computer.

-   **Purpose:** To listen for serial commands from the macropad (sent from Layers 2 and 3) and execute actions on the host OS.
-   **How it works:**
    1.  The script continuously scans for an active serial connection to the macropad.
    2.  It loads the `config.json` file into memory to understand what each command means.
    3.  When it receives a command (e.g., `L2:C1R2`), it looks up the corresponding action in the loaded config data.
    4.  It then executes that action. This could be opening a web URL, running a shell command, or launching an application (`.exe`, `.app`, etc.).
-   **Dependencies:** The script requires `pyserial` and other libraries. These can be installed via pip:
    ```bash
    pip install -r host-listener/requirements.txt
    ```

---

## 5. Complete User Workflow

This guide outlines the end-to-end process from assembly to full functionality.

1.  **Build the Hardware:** Fabricate the PCBs using the provided KiCad files and solder all components as specified in the BOM. Assemble the top and bottom boards using M2.5 standoffs.
2.  **Flash the Firmware:**
    -   Install Visual Studio Code with the PlatformIO extension.
    -   Open the `firmware/` directory in VSCode.
    -   Connect the macropad to your computer via USB-C.
    -   Use the PlatformIO "Upload" task to build and flash the firmware to the ESP32-S3.
3.  **Create a Configuration:**
    -   Open the `web-configurator/index.html` file in your browser.
    -   Design your layouts. Assign functions to keys on Layers 1, 2, and 3. Set your desired lighting.
    -   Click the "Export JSON" button to download your `config.json` file. Save it to a known location (e.g., `Documents/Macropad/config.json`).
4.  **Run the Host Listener:**
    -   Open a terminal or command prompt.
    -   Navigate to the `host-listener/` directory.
    -   Install the required Python packages: `pip install -r requirements.txt`.
    -   Run the listener application: `python listener.py`.
    -   The first time you run the script, its window will appear. Click "Browse..." and locate the `config.json` file you saved in the previous step. The application will remember this location for future runs.
    -   The application will automatically connect to the macropad. You can now minimize the window, and it will run in the system tray.
5.  **Use Your Macropad:** You're all set. The macropad will now execute the actions you defined. Key presses on Layers 0/1 will work instantly. Key presses on Layers 2/3 will work as long as the Host Listener is running.
