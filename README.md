# 🔋 Smart 9V Battery Harvester

An open-source, 6-slot hardware and software system designed to aggregate voltage from multiple 9V batteries into a single regulated 12V output. Featuring real-time ESP32 telemetry, local OLED status monitoring, and a responsive web-based Bluetooth dashboard complete with a built-in interactive simulation mode.

---

## 🚀 Features
* **6-Slot Parallel Architecture:** Isolated inputs using 1N5819 Schottky diodes to prevent cross-charging and back-feeding.
* **ESP32 Telemetry:** Real-time multi-channel ADC voltage sampling using $20\text{ k}\Omega / 10\text{ k}\Omega$ resistor dividers.
* **Web Bluetooth Dashboard (`index.html`):** Live Chart.js visualization tracking individual slot voltages and bus average.
* **Interactive Simulation Mode:** Test the dashboard locally in your browser without physical hardware by toggling fresh, depleted, or empty battery states.
* **Local OLED Display:** SSD1306 display providing instant on-device status metrics.

---

## 📋 Bill of Materials
See the full [Bill of Materials (BOM)](./bom.md) for a detailed component breakdown.

---

## 🌐 Web Dashboard & Simulation
You can access the live web dashboard hosted via GitHub Pages or run `index.html` locally in any Web Bluetooth-compatible browser (such as Google Chrome or Microsoft Edge). Click **Start Simulation Mode** to test telemetry and visual charts instantly.

---

## 📜 Licensing
* **Software:** Licensed under the [MIT License](LICENSE-SOFTWARE).
* **Hardware:** Licensed under the [CERN Open Hardware Licence (CERN-OHL-P)](LICENSE-HARDWARE).
