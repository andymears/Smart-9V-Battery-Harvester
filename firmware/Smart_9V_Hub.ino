#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- OLED Setup ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- BLE UUIDs ---
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// --- ADC Pin Mapping (ESP32 DevKit) ---
const int numSlots = 4;
const int adcPins[numSlots] = {32, 33, 34, 35}; // ADC1 pins recommended with Wi-Fi/BLE

// Calibration constants for 20k/10k voltage divider
const float R_TOP = 20000.0; // 20k ohm
const float R_BOTTOM = 10000.0; // 10k ohm
const float ADC_VREF = 3.3;  // ESP32 ADC reference voltage
const int ADC_MAX = 4095;    // 12-bit ADC

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    }
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        pServer->startAdvertising(); // Restart advertising when disconnected
    }
};

void setup() {
    Serial.begin(115200);

    // Initialize I2C and SSD1306 OLED
    Wire.begin(21, 22); // SDA = GPIO 21, SCL = GPIO 22
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        while(true);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Smart 9V Harvester");
    display.display();
    delay(1000);

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
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
    Serial.println("BLE Device active, waiting for connections...");
}

void loop() {
    float voltages[numSlots];
    float maxVoltage = 0.0;
    float activeSum = 0.0;
    int activeCount = 0;

    // Read and calculate voltages for all battery slots
    for (int i = 0; i < numSlots; i++) {
        int rawValue = analogRead(adcPins[i]);
        // Convert ADC reading to pin voltage
        float pinVoltage = (rawValue / (float)ADC_MAX) * ADC_VREF;
        // Reverse voltage divider formula to find actual battery voltage
        float batteryVoltage = pinVoltage * ((R_TOP + R_BOTTOM) / R_BOTTOM);
        
        // Filter out noise or empty slots (under ~4V means no battery or dead cell)
        if (batteryVoltage < 4.0) {
            voltages[i] = 0.0;
        } else {
            voltages[i] = batteryVoltage;
            activeSum += batteryVoltage;
            activeCount++;
        }
    }

    // Common bus average (or highest diode drop approximation)
    float busAvg = (activeCount > 0) ? (activeSum / activeCount) : 0.0;

    // Update Local OLED Display
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("--- 9V Smart Hub ---");
    for (int i = 0; i < numSlots; i++) {
        display.setCursor(0, 12 + (i * 10));
        display.print("Slot "); display.print(i + 1); display.print(": ");
        if (voltages[i] > 4.0) {
            display.print(voltages[i], 2); display.println(" V");
        } else {
            display.println("Empty");
        }
    }
    display.setCursor(0, 52);
    display.print("Bus Avg: "); display.print(busAvg, 2); display.println(" V");
    display.display();

    // Broadcast over BLE if connected
    if (deviceConnected) {
        String payload = String(voltages[0], 2) + "," + 
                         String(voltages[1], 2) + "," + 
                         String(voltages[2], 2) + "," + 
                         String(voltages[3], 2) + "|" + 
                         String(busAvg, 2);
        
        pCharacteristic->setValue(payload.c_str());
        pCharacteristic->notify();
    }

    delay(2000); // Send update every 2 seconds
}
