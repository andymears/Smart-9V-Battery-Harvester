# Bill of Materials (BOM) - Smart 9V Battery Harvester (6-Slot)

This document lists all the hardware components required to build the 6-slot Smart 9V Battery Harvester.

| Item Category | Component Description / Part Number | Qty | Purpose / Notes |
| :--- | :--- | :---: | :--- |
| **Microcontroller** | ESP32 DevKit NodeMCU (38-pin version) | 1 | Handles ADC telemetry sampling, OLED rendering, and BLE GATT server. |
| **Display** | SSD1306 0.96" OLED Display (I2C, 128x64) | 1 | Local real-time status output (`SDA: GPIO 21`, `SCL: GPIO 22`). |
| **Battery Interface** | 9V Battery Snaps / Plastic Holders | 6 | Physical bays for holding standard 9V batteries. |
| **Isolation Diodes** | 1N5819 Schottky Diode (40V, 1A) | 6 | Prevents back-feeding and isolates parallel inputs onto the common bus. |
| **Voltage Dividers** | Resistor: $20\text{ k}\Omega$ (1/4W, 1%) | 6 | Top leg of the ADC voltage divider network. |
| **Voltage Dividers** | Resistor: $10\text{ k}\Omega$ (1/4W, 1%) | 6 | Bottom leg of the ADC voltage divider network (safe 0–3V range). |
| **Power Regulation** | XL6009 DC-DC Adjustable Boost Converter | 1 | Steps up the combined variable 9V input bus to a steady 12V output. |
| **Logic Regulation** | 3.3V Step-Down Buck Regulator | 1 | Provides clean 3.3V power rail for the ESP32 and OLED VCC. |
| **Prototyping / Wiring** | Perforated PCB & Hookup Wire | As req. | Connecting traces, ground bus strips, and terminal blocks. |
| **Enclosure** | 3D Printable Enclosure Base & Lid (`.stl`) | 1 set | Custom housing for desk or wall mounting. |
