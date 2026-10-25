/*
 * Smart 9V Battery Harvester Firmware
 * Board: ESP32 Dev Module (38-pin DevKit)
 * License: MIT
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ESP32 Hardware ADC Pins (ADC1 channels safe to use with Bluetooth enabled)
const int batteryPins[6] = {36, 39, 34, 35, 32, 33};

// Voltage Divider Calibration Parameters
// R1 = 22k Top Resistor, R2 = 10k Bottom Resistor
const float R1 = 22000.0;
const float R2 = 10000.0;
const float ADC_MAX = 4095.0;
const float V_REF = 3.3; // ESP32 ADC Reference Voltage
const float LOW_VOLTAGE_CUTOFF = 5.0; // Voltage floor cutoff for depleted battery

// BLE UUID Configuration
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// BLE Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnected(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("[BLE] Client Device Connected!");
    };
    void onDisconnected(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("[BLE] Client Device Disconnected!");
      // Restart advertising so client can reconnect seamlessly
      BLEDevice::startAdvertising();
    }
};

// Moving average sampling filter for smooth ADC readings
float readFilteredVoltage(int pin) {
  long sum = 0;
  const int samples = 16;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(100);
  }
  float rawAvg = (float)sum / samples;
  float vPin = (rawAvg / ADC_MAX) * V_REF;
  return vPin * ((R1 + R2) / R2);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Smart 9V Battery Harvester...");

  // Initialize I2C Bus (SDA = GPIO 21, SCL = GPIO 22)
  Wire.begin(21, 22);

  // Initialize SSD1306 Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 OLED initialization failed!"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Smart 9V Harvester"));
  display.println(F("Initializing ADC..."));
  display.println(F("BLE: Starting..."));
  display.display();
  delay(1500);

  // Configure ADC Pins
  for (int i = 0; i < 6; i++) {
    pinMode(batteryPins[i], INPUT);
  }

  // Initialize BLE Stack
  BLEDevice::init("9V_Harvester");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Advertising started. Ready for connections!");
}

void loop() {
  float voltages[6];
  String jsonPayload = "{";

  // Read and process all 6 battery channels
  for (int i = 0; i < 6; i++) {
    voltages[i] = readFilteredVoltage(batteryPins[i]);
    String statusStr = (voltages[i] < LOW_VOLTAGE_CUTOFF) ? "EMPTY" : "OK";

    // Format telemetry string as {"s1":{"v":8.4,"st":"OK"}, ...}
    jsonPayload += "\"s" + String(i + 1) + "\":{\"v\":" + String(voltages[i], 2) + ",\"st\":\"" + statusStr + "\"}";
    if (i < 5) jsonPayload += ",";
  }
  jsonPayload += "}";

  // Update Local SSD1306 OLED Screen
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("9V Harvester Telemetry"));
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  for (int i = 0; i < 6; i++) {
    int col = (i < 3) ? 0 : 64;
    int row = 14 + ((i % 3) * 16);
    
    display.setCursor(col, row);
    display.print("S"); display.print(i + 1); display.print(":");
    
    if (voltages[i] < LOW_VOLTAGE_CUTOFF) {
      display.print("DEAD ");
    } else {
      display.print(voltages[i], 1); display.print("V");
    }
  }

  // BLE Connection Indicator at bottom right of screen
  display.setCursor(96, 52);
  display.print(deviceConnected ? "[BLE]" : "[---]");
  display.display();

  // Send BLE Telemetry Notification
  if (deviceConnected) {
    pCharacteristic->setValue(jsonPayload.c_str());
    pCharacteristic->notify();
    Serial.println("[BLE Stream] " + jsonPayload);
  }

  delay(1000); // 1 Hz Telemetry Refresh Rate
}
