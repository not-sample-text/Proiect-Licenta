### 1. Top Board (User Interface)

_Contains the inputs and displays that the user interacts with directly._

| Component Name     | Qty | Value / Specification | Footprint / Part Detail   | Purpose / Function                                                            |
| :----------------- | :-: | :-------------------- | :------------------------ | :---------------------------------------------------------------------------- |
| **Keyswitch**      | 12  | Tactile Switch        | `MX_Hotswap_1.00u`        | **User Input:** Provides momentary electrical connection for key presses.     |
| **Diode**          | 12  | 1N4148                | `D_DO-35_SOD27`           | **Matrix Protection:** Prevents "ghosting" and ensures current flows one way. |
| **Rotary Encoder** |  1  | EC11 w/ Switch        | `Encoder_EC11_MX`         | **User Input:** Continuous rotational input and momentary push-button.        |
| **OLED Display**   |  1  | SSD1306 128x32        | `OLED_128x32`             | **Display Output:** Shows layers, battery status, and connection info.        |
| **SPDT Switch**    |  1  | Slide Switch          | `SLW8645745ARAND`         | **Mode Selection:** Hard switch for On/Off or Mode toggling.                  |
| **Pin Header**     |  1  | 1x15 Male Header      | `PinHeader_1x15_Vertical` | **Connection:** Connects the Top Board to the Bottom Board.                   |

---

### 2. Bottom Board (Logic & Power)

_Contains the microcontroller, wireless components, and RGB lighting._

| Component Name        | Qty | Value / Specification | Footprint / Part Detail   | Purpose / Function                                                              |
| :-------------------- | :-: | :-------------------- | :------------------------ | :------------------------------------------------------------------------------ |
| **Pin Socket**        |  1  | 1x15 Female Socket    | `PinSocket_1x15_Vertical` | **Connection:** Mates with the Top Board for a modular assembly.                |
| **Microcontroller**   |  1  | UM ESP32-S3 Pro       | `DU1 / ProS3_TH`          | **Main Processor:** Firmware execution, I/O, and WiFi/BLE radio.                |
| **Antenna**           |  1  | 2.4 GHz Flexible      | `AC10200-100` / U.FL      | **Wireless:** Transmits and receives Bluetooth/WiFi signals.                    |
| **RGB LED**           | 10  | SK6812MINI            | `PLCC4_3.5x3.5mm`         | **Visuals:** Addressable, full-color underglow lighting.                        |
| **Pull-Up Resistor**  | 10  | 10k $\Omega$          | `R_0805_HandSolder`       | **Stabilization:** Ensures input pins stay at a high logic state when inactive. |
| **Limiting Resistor** | 10  | 470 $\Omega$          | `R_0805_HandSolder`       | **Protection:** Limits current flow to LEDs or data lines.                      |
| **Capacitor**         |  1  | 100nF                 | `C_0805_HandSolder`       | **Decoupling:** Filters electrical noise near the power pins.                   |

---

### 3. Mechanical Hardware

_Physical components used to assemble the case and PCBs._

| Component Name | Qty | Specification | Type                  | Purpose / Function                                            |
| :------------- | :-: | :------------ | :-------------------- | :------------------------------------------------------------ |
| **Standoffs**  |  4  | M2.5 x 11mm   | Brass (Female/Female) | **Spacing:** Separates and secures the Top and Bottom boards. |
| **Screws**     |  4  | M2.5 x 4mm    | Brass / Steel         | **Fastening:** Secures the PCBs to the standoffs.             |
