# ESP32-S3 Macropad - Thesis Project

A custom 12-key mechanical macropad based on the ESP32-S3, featuring programmable keys, a rotary encoder, an OLED display, and dual-mode connectivity (USB/Bluetooth).

![Top PCB Render](Documentatie/top_pcb_1.png)

---

## About This Project

This repository contains the hardware design files for a versatile and programmable macropad created as a university thesis project. It's designed for productivity, allowing users to create custom shortcuts, macros, and function layers to streamline their workflows.

**For detailed technical documentation, please see the [Documentation File](docs/Documentatie.md) (Romanian).**

## Project Status

:warning: **In Development:** The hardware and web configurator are complete. The firmware is under active development. Guides for assembly and usage will be updated as the project progresses.

## Features

- **12 Hot-Swappable Mechanical Keys:** Customize the feel of your macropad with any MX-style switches.
- **Web-Based Configuration:** An intuitive browser-based UI allows you to remap keys and create macros without needing to write any code.
- **Rotary Encoder:** Includes a push-button for intuitive menu navigation, volume control, and more.
- **OLED Display:** A 0.91" SSD1306 screen provides real-time feedback on active layers, connection status, and battery level.
- **Dual Connectivity:** Seamlessly switch between a wired USB-C connection and wireless Bluetooth/BLE HID.
- **Programmable Layers:** The firmware reads a user-generated config file to support multiple layers for different applications and use cases.
- **Power Management:** An on-board switch allows the device to enter a low-power state to conserve battery.

## Software Ecosystem

This project uses a 2-part architecture to manage configuration:

### 1. Web Configurator

Located in the [`web-configurator/`](web-configurator/) directory, this is a standalone HTML/JS application.

-   **Purpose:** Allows the user to visually design their key layouts, shortcuts, and macros across multiple layers.
-   **Output:** Generates a `config.json` file containing the complete custom configuration. This file is then placed on the host computer for the firmware to access (details on synchronization will be provided later).

### 2. Firmware

The firmware is responsible for reading the `config.json` and applying the defined behavior to the macropad. It acts as a standard HID Keyboard, translating user-defined layouts into keypresses that the operating system can understand.

## Hardware Overview

- **Microcontroller:** Unexpected Maker ProS3 (ESP32-S3)
- **Design:** A two-board design separates the user interface components (keys, encoder) from the main controller board.
- **Components:**
  - 12x MX-compatible hot-swap sockets
  - EC11-compatible rotary encoder
  - SSD1306 128x32 I2C OLED display
  - 1N4148 diodes for anti-ghosting

The full schematics, PCB layouts, and (eventually) Bill of Materials are located in this `Documentatie` folder.

## Firmware

The firmware for this macropad is being custom-written in C/C++ to leverage the specific features of the ESP32-S3. It will not use existing frameworks like QMK or ZMK. Key development goals include implementing the layer system, HID communication (USB & BLE), and OLED screen management.

## Getting Started

1.  **Fabricate PCBs:** Use the KiCad files in this repository to generate Gerbers and order the PCBs.
2.  **Assemble Hardware:** Solder the components according to the PCB layout. A detailed assembly guide is available in the [documentation](Documentatie/Documentatie.md).
3.  **Flash Firmware:** Once the firmware is released, it can be flashed onto the ProS3 board. Instructions will be provided.

## Future Plans

- Finalize and release the custom firmware.
- Generate a detailed Bill of Materials (BOM).
- Design and release files for a 3D-printable enclosure.

## Acknowledgements

- **[Unexpected Maker](https://unexpectedmaker.com/)** for the excellent ProS3 board.
- **[ScottoKeebs](https://github.com/joe-scotto/scottokeebs)** for the KiCad libraries and footprints for mechanical keyboards.
- The mechanical keyboard community for endless inspiration.

## License

The hardware design files are provided as-is. This project includes components from third-party libraries which are subject to their own licenses.
