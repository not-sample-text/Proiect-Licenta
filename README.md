# Repository Archived

This repository represents the final version submitted for my Bachelor's Thesis.

Active development for this project has moved to [ApexPad](https://github.com/not-sample-text/ApexPad). Please refer to the new repository for the most recent updates, bug fixes, and feature development.

---

# PROS3 Macropad: A Fully-Programmable Custom Mechanical Macropad

The PROS3 Macropad is a versatile, open-source 12-key macropad built around the powerful ESP32-S3. Designed as a university thesis project, it features a rotary encoder, OLED display, RGB underglow, and a complete software ecosystem for deep customization.

---

## Gallery

A collection of renders showcasing the hardware.

|                 Top Plate                  |                     Top Plate (Angled)                     |
| :----------------------------------------: | :--------------------------------------------------------: |
| ![Top Plate](docs/images/render-plate.png) | ![Top Plate (Angled)](docs/images/render-plate-angled.png) |

|                 Top View                  |                        Top (Angled)                        |
| :---------------------------------------: | :--------------------------------------------------------: |
| ![Top Render](docs/images/render-top.png) | ![Top Render (Angled)](docs/images/render-top-angled.png)! |

|                  Bottom Render                  |                         Bottom (Angled)                         |
| :---------------------------------------------: | :-------------------------------------------------------------: |
| ![Bottom Render](docs/images/render-bottom.png) | ![Bottom Render (Angled)](docs/images/render-bottom-angled.png) |

---

## About The Project

This repository contains the complete engineering files (hardware, firmware, and software) for a highly programmable macropad designed to streamline digital workflows.

The project is built on three pillars:

1. **Hardware:** A custom two-part PCB design with a 3D-printable case. It's designed for assembly by hobbyists with through-hole components and hot-swap sockets.
2. **Firmware:** Custom C++ code running on the ESP32-S3. It manages input, the OLED display, lighting, and communication with the host computer.
3. **Software Suite:** A powerful combination of a static web-based configurator (for visually creating keymaps) and a headless Python-based background daemon (for executing complex OS-level actions via a system tray UI).

This architecture allows the macropad to go beyond simple keystrokes and act as a powerful automation tool.

**For a complete technical deep-dive, please see the [Full Technical Documentation](docs/Documentation.md).**
**(Pentru documentația în limba română, consultați [acest document](docs/Documentatie.md)).**

---

## Features

- **12 Hot-Swappable Mechanical Keys:** Customize your macropad with any MX-style switches, no soldering required.
- **Rotary Encoder with Push-Button:** Perfect for volume control, scrolling through timelines, or cycling through layers.
- **OLED Display:** Get real-time feedback on your current layer, connection status, and more.
- **Powerful Layering System:** Switch between 4 distinct layers to multiply your available keys.
- **Dual-Mode Actions:**
    - **HID Mode:** Send standard keystrokes and shortcuts that work on any OS without drivers.
    - **Host Control Mode:** Trigger complex actions on your computer—like launching native apps or executing shell scripts—using the auto-discovering background daemon.
- **Web-Based Configurator:** A zero-dependency, browser-based UI for remapping keys, creating macros, and configuring lighting. Features a guided interactive tour and smart JSON export.
- **RGB Underglow:** 10 addressable RGB LEDs for brilliant lighting effects, managed by FastLED.
- **Dual Connectivity:** Seamlessly switch between a wired USB-C connection and wireless Bluetooth/BLE HID.

---

## Getting Started

1.  **Build the Hardware:** Use the KiCad files in the [`hardware/`](hardware/) directory to fabricate the PCBs and source the components from the Bill of Materials.
2.  **Flash the Firmware:** Use PlatformIO to compile and upload the firmware from the [`firmware/`](firmware/) directory to the ESP32-S3.
3.  **Configure Your Layout:** Access the Web Configurator (locally or via GitHub Pages) to design your custom keymaps and export the `config.json` file to your Downloads folder.
4.  **Run the Host Daemon:** Download the standalone executable from the Releases page (or run the Python script). It automatically connects to the hardware, detects your config file, and runs invisibly in your system tray to unlock the command and launcher layers.

For a detailed, step-by-step guide, please refer to the **[Complete User Workflow section in the documentation](docs/Documentation.md#5-complete-user-workflow)**.

---

## Acknowledgements

This project was made possible by the following open-source projects and communities:

- **[ScottoKeebs](https://github.com/joe-scotto/scottokeebs)**: For the KiCad libraries and footprints for mechanical keyboards.
- **The Mechanical Keyboard Community**: For providing a constant source of inspiration and knowledge.

## License

The hardware and software are provided as-is under a permissive license. This project includes components from third-party libraries which are subject to their own licenses. Please see the individual source files for more details.
