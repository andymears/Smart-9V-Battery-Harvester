#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// OLED Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// BLE UUIDs (Generated for this project)
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Battery ADC Pins & Config
const int batteryPins[] = {32, 33, 34, 35};
const int numBatteries = sizeof(batteryPins) / sizeof(batteryPins[0]);
const float voltageDividerRatio = 3.0; // 20k / 10k divider

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Initializing BLE..."));
  display.display();

  // Initialize BLE
  BLEDevice::init("Smart-9V-Hub");
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
  pAdvertising->addServiceUuid(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connection issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  // Configure ADC pins
  for (int i = 0; i < numBatteries; i++) {
    pinMode(batteryPins[i], INPUT);
  }
}

void loop() {
  float totalBankVoltage = 0;
  int activeBatteries = 0;
  float batteryVoltages[numBatteries];

  // Read ADC and compute voltages
  for (int i = 0; i < numBatteries; i++) {
    int rawValue = analogRead(batteryPins[i]);
    float pinVoltageMv = (rawValue / 4095.0) * 3300.0;
    float batteryVoltage = (pinVoltageMv * voltageDividerRatio) / 1000.0;

    if (batteryVoltage > 4.0) {
      batteryVoltages[i] = batteryVoltage;
      totalBankVoltage += batteryVoltage;
      activeBatteries++;
    } else {
      batteryVoltages[i] = 0.0;
    }
  }

  float avgBankVoltage = (activeBatteries > 0) ? (totalBankVoltage / activeBatteries) : 0.0;

  // Format JSON-like string payload for BLE: e.g., "8.12,7.95,6.80,0.00|7.62"
  String payload = "";
  for (int i = 0; i < numBatteries; i++) {
    payload += String(batteryVoltages[i], 2);
    if (i < numBatteries - 1) payload += ",";
  }
  payload += "|" + String(avgBankVoltage, 2);

  // Update BLE characteristic value if connected
  if (deviceConnected) {
    pCharacteristic->setValue(payload.c_str());
    pCharacteristic->notify();
  }

  // --- UPDATE OLED DISPLAY ---
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("BLE Hub | Active: "));
  display.print(activeBatteries);

  display.setCursor(0, 10);
  display.print(F("-------------------------"));

  for (int i = 0; i < numBatteries; i++) {
    display.setCursor(0, 20 + (i * 10));
    display.print(F("Slot "));
    display.print(i + 1);
    display.print(F(": "));
    if (batteryVoltages[i] > 4.0) {
      display.print(batteryVoltages[i], 2);
      display.print(F("V"));
    } else {
      display.print(F("Empty"));
    }
  }

  display.setCursor(0, 54);
  display.print(F("Bus Avg: "));
  display.print(avgBankVoltage, 2);
  display.print(F("V "));
  display.print(deviceConnected ? F("[BLE]") : F("[-]"));

  display.display();
  delay(2000);
}
