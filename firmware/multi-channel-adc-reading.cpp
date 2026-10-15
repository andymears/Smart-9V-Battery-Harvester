// Define the ESP32 ADC pins connected to your battery slots
const int batteryPins[] = {32, 33, 34, 35};
const int numBatteries = sizeof(batteryPins) / sizeof(batteryPins[0]);

// Voltage divider ratio calculation: 
// Top Resistor (R1) = 20k, Bottom Resistor (R2) = 10k
// Ratio = (R1 + R2) / R2 = (20 + 10) / 10 = 3.0
const float voltageDividerRatio = 3.0;

// ESP32 ADC reference voltage is typically 3.3V, but analogReadMilliVolts handles internal calibration well.
// If using standard analogRead(), we map 0-4095 to 0-3300mV.

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Smart 9V Battery Hub - ADC Telemetry Initialized");

  // Configure ADC pins as inputs
  for (int i = 0; i < numBatteries; i++) {
    pinMode(batteryPins[i], INPUT);
  }
}

void loop() {
  Serial.println("----------------------------------------");
  float totalBankVoltage = 0;
  int activeBatteries = 0;

  for (int i = 0; i < numBatteries; i++) {
    // Read raw ADC value (0 - 4095 on ESP32)
    int rawValue = analogRead(batteryPins[i]);
    
    // Convert raw reading to millivolts at the ESP32 pin (approx. 0 to 3300mV)
    // ESP32 ADC has non-linearity at extremes, but this is great for a baseline.
    float pinVoltageMv = (rawValue / 4095.0) * 3300.0;
    
    // Scale back up using the voltage divider ratio to get the actual battery voltage
    float batteryVoltage = (pinVoltageMv * voltageDividerRatio) / 1000.0; // convert to Volts

    Serial.print("Battery Slot ");
    Serial.print(i + 1);
    Serial.print(": ");
    
    // Basic threshold to check if a battery is actually inserted (> 4.0V)
    if (batteryVoltage > 4.0) {
      Serial.print(batteryVoltage, 2);
      Serial.println(" V");
      totalBankVoltage += batteryVoltage;
      activeBatteries++;
    } else {
      Serial.println("Empty / Dead (< 4V)");
    }
  }

  Serial.print("Active Cells: ");
  Serial.println(activeBatteries);
  Serial.print("Combined Bank Parallel Voltage Pool: ");
  Serial.print(totalBankVoltage / max(1, activeBatteries), 2); // Approximate parallel bus voltage
  Serial.println(" V (avg)");

  delay(2000); // Read every 2 seconds
}
