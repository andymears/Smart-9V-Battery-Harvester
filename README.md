# Smart 9V Battery Management Hub (V1)
An open-source hardware project that harvests remaining energy from used alkaline 9V smoke detector batteries, combining them intelligently into a stabilized 12V automotive socket for universal USB charging and device power.

## 🛠 Project Architecture
- **Input:** 5-6 used alkaline 9V batteries in parallel, isolated via Schottky diodes.
- **Sensing:** ESP32 microcontroller with individual ADC voltage dividers.
- **Regulation:** XL6009 DC-DC Boost Converter stepping the dropping cell voltage up to 12.0V.
- **Output:** Standard 12V automotive cigarette lighter socket (compatible with standard car USB adapters).
- **Telemetry:** I2C OLED local display + Bluetooth Low Energy (BLE) Web Dashboard.

## 📋 Bill of Materials (BOM)
| Component | Purpose | Qty |
| :--- | :--- | :--- |
| ESP32 DevKit V1 | Microcontroller, ADC sensing, and BLE telemetry | 1 |
| 0.96" I2C OLED (SSD1306) | Local status display | 1 |
| XL6009 Boost Converter Module | Boosts input rail to steady 12V | 1 |
| 12V Panel Mount Auto Socket | Universal output receptacle | 1 |
| 1N5819 Schottky Diodes | Individual battery back-feed protection | 5-6 |
| 10kΩ & 20kΩ Resistors | Voltage dividers for ESP32 ADC pins | 10-12 |
| 9V Battery Snaps / Holders | Physical battery bays | 5-6 |
