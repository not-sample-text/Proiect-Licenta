# ESP32 Macropad - Proiect Licență

A custom 12-key mechanical macropad built around the ESP32-S3, featuring programmable mechanical switches, a rotary encoder, OLED display, and dual connectivity modes (USB/Bluetooth).

![Top PCB View 1](Documentatie/top_pcb_1.png)
![Top PCB View 2](Documentatie/top_pcb_2.png)

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Components](#hardware-components)
- [PCB Design](#pcb-design)
- [Firmware](#firmware)
- [Connectivity](#connectivity)
- [Gallery](#gallery)
- [License](#license)

## 🔍 Overview

This project is a productivity-focused macropad designed for customizable workflows. Built with the Unexpected Maker ProS3, it combines mechanical switches with smart features like battery monitoring via NeoPixel LED, an OLED display for visual feedback, and seamless switching between wired and wireless modes.

## ✨ Features

- **12 Mechanical Keys**: Hot-swappable MX-style mechanical switches
- **Rotary Encoder**: EC11-compatible encoder with push-button for volume control and menu navigation
- **OLED Display**: 128x32 SSD1306 OLED screen (0.91" diagonal) showing:
  - Battery level indicator
  - Active layer information
  - Last pressed key/shortcut
  - Connection status
- **Dual Connectivity**: Toggle between USB HID and Bluetooth/BLE HID modes
- **Power Management**: SPDT switch for low-power mode
- **Battery Indicator**: NeoPixel LED with color-coded battery status:
  - Green (>80%): Solid green
  - Yellow-Orange (20-80%): Gradient color
  - Red (<20%): Flashing red
- **Three Programmable Layers**:
  1. Extended function keys (F13-F24) for application shortcuts
  2. Common shortcuts (CTRL+Z, ALT+TAB, etc.)
  3. Script execution keys

## 🔧 Hardware Components

### Main Components

- **Unexpected Maker ProS3**: ESP32-S3 based development board with USB-C and built-in battery charging
- **Rotary Encoder**: EC11-compatible encoder with integrated switch
- **OLED Display**: SSD1306 128x32 I2C OLED display
- **Mechanical Switches**: 12x MX-compatible hot-swap sockets
- **SPDT Switch**: ON-OFF toggle for power management

### Passive Components

- **Diodes**: 1N4148 fast-switching THT diodes for anti-ghosting protection
- **Pull-up Resistors**: 10kΩ SMD 0805 resistors
- **Decoupling Capacitor**: 100nF SMD 1206 capacitor

### Pin Reference

![ProS3 Pin Reference](Documentatie/ProS3_Pin_Reference.png)

## 🛠 PCB Design

The project consists of a two-layer PCB design created in KiCad:

### Top Board (Main Control)

The top board houses the user interface components:

- 12 mechanical switch sockets in a 3x4 matrix layout
- Rotary encoder position
- OLED display connector
- Mode selection switch
- Header pins for connecting to the bottom board

![Top Schematic](Documentatie/top_schematic.pdf)
![Top Layout](Documentatie/top_layout.pdf)

### Bottom Board (Controller)

The bottom board contains the main controller and power management:

- Unexpected Maker ProS3 footprint
- Battery monitoring circuitry
- Voltage regulation components
- Protection diodes
- Mounting holes for assembly

![Bottom PCB](Documentatie/bottom_pcb.png)
![Bottom Schematic](Documentatie/bottom_schematic.pdf)
![Bottom Layout](Documentatie/bottom_layout.pdf)

## 💻 Firmware

### Battery Monitoring

The firmware includes intelligent battery monitoring with visual feedback through the onboard NeoPixel LED:

```cpp
// Battery voltage thresholds for 3.7V LiPo
const float VOLTAGE_FULL = 4.20;  // 100%
const float VOLTAGE_EMPTY = 3.30; // 0%
```

**Color Indicators:**

- **Green**: Battery level above 80%
- **Gradient (Green→Yellow→Red)**: Battery level between 20-80%
- **Flashing Red**: Battery level below 20% (critical)

The system reads battery voltage through the A13 analog pin and updates the LED status every second.

### Dependencies

- `Adafruit_NeoPixel`: For controlling the RGB LED

## 🔌 Connectivity

The macropad supports two main communication protocols:

### USB HID

- Standard USB connection via USB-C port
- Low latency, no pairing required
- Suitable for desktop/workstation use

### Bluetooth/BLE HID

- Wireless connectivity for portable use
- Supports multiple device pairing
- Battery-powered operation

Users can switch between modes using the rotary encoder and monitor the active connection on the OLED display.

## 📸 Gallery

### PCB Designs

![Top PCB Design 1](Documentatie/top_pcb_1.png)
_Top PCB - Front view showing switch layout and component placement_

![Top PCB Design 2](Documentatie/top_pcb_2.png)
_Top PCB - Back view showing traces and connections_

![Bottom PCB Design](Documentatie/bottom_pcb.png)
_Bottom PCB - Controller board with ProS3 footprint_

### Schematics

- [Top Board Schematic](Documentatie/top_schematic.pdf)
- [Bottom Board Schematic](Documentatie/bottom_schematic.pdf)

### PCB Layouts

- [Top Board Layout](Documentatie/top_layout.pdf)
- [Bottom Board Layout](Documentatie/bottom_layout.pdf)

## 📦 Project Structure

```
Proiect-Licenta/
├── Code/
│   └── battery_neopixel/
│       └── battery_neopixel.ino      # Battery monitoring firmware
├── Documentatie/
│   ├── Documentatie.md                # Project documentation (Romanian)
│   ├── ProS3_Pin_Reference.png       # ESP32-S3 pinout reference
│   ├── top_pcb_1.png                 # Top PCB render (front)
│   ├── top_pcb_2.png                 # Top PCB render (back)
│   ├── bottom_pcb.png                # Bottom PCB render
│   ├── top_schematic.pdf             # Top board schematic
│   ├── bottom_schematic.pdf          # Bottom board schematic
│   ├── top_layout.pdf                # Top board layout
│   └── bottom_layout.pdf             # Bottom board layout
├── kicad/
│   ├── top/                          # Top PCB KiCad project files
│   └── bottom/                       # Bottom PCB KiCad project files
└── scottokeebs_kicad_libs/           # Custom KiCad libraries for mechanical keyboards
```

## 🚀 Getting Started

### Hardware Assembly

1. Order PCBs using the gerber files from the KiCad project
2. Solder passive components (diodes, resistors, capacitors) on both boards
3. Install hot-swap sockets on the top board
4. Solder header pins between top and bottom boards
5. Mount the ProS3 on the bottom board
6. Connect the OLED display and rotary encoder
7. Install mechanical switches

### Firmware Upload

1. Install Arduino IDE with ESP32 board support
2. Install required libraries:
   - Adafruit NeoPixel
3. Open `Code/battery_neopixel/battery_neopixel.ino`
4. Select "Unexpected Maker ProS3" as the board
5. Upload the firmware

### Software Configuration

_Firmware for keyboard functionality coming soon_

## 🤝 Contributing

This is a university thesis project (Proiect Licență). Suggestions and feedback are welcome!

## 📄 License

This project includes components from [ScottoKeebs](https://github.com/joe-scotto/scottokeebs), which are used under their respective licenses.

## 🙏 Acknowledgments

- [Unexpected Maker](https://unexpectedmaker.com/) for the ProS3 board
- [ScottoKeebs](https://github.com/joe-scotto/scottokeebs) for KiCad keyboard libraries and footprints
- The mechanical keyboard community for inspiration and resources

---

**Project Status**: In Development 🚧

_Created as part of a Bachelor's Thesis (Licență) project_
