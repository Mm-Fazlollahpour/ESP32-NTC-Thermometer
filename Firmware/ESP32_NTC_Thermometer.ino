#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic_2 = NULL;
BLEDescriptor *pDescr;
BLE2902 *pBLE2902;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_2 "3da2e68d-283c-4617-b03e-160835de5d5d"

Adafruit_SSD1306 IES(128,64,&Wire,-1);
int led1 = 15; //red
int led2 = 4;  //green
int opout = 13;
int threshold = 26;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t temperature = 0;
int pChar2_value_int = 0; // Declare pChar2_value_int as a global variable

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class CharacteristicsCallBack: public BLECharacteristicCallbacks {
 void onWrite(BLECharacteristic *pChar) override {
    String pChar2_value_string = pChar->getValue().c_str();
    pChar2_value_int = pChar2_value_string.toInt(); // Update the global variable
    Serial.println("pChar2: " + String(pChar2_value_int));
  }
};

int finaltemp[2];
void setup() {
  Serial.begin(9600);
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(opout,INPUT);

  IES.begin(SSD1306_SWITCHCAPVCC,0x3C);
  IES.setTextColor(WHITE);
  IES.clearDisplay();

  BLEDevice::init("ESP32");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic_2 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_2,
                      BLECharacteristic::PROPERTY_READ  |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pDescr = new BLEDescriptor((uint16_t)0x2901);
  pDescr->setValue("1024");
  pCharacteristic->addDescriptor(pDescr);

  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  pCharacteristic->addDescriptor(pBLE2902);
  pCharacteristic_2->addDescriptor(new BLE2902());
  pCharacteristic_2->setCallbacks(new CharacteristicsCallBack()); // Set the callback

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    float voltage = analogRead(opout);
    float temperature= 0;
    if (deviceConnected) {
        temperature = voltage * 3.3/4096 * (-53.561) + 149.97;
        int temperatureInt = int(temperature);
        pCharacteristic->setValue(temperatureInt);

        pCharacteristic->notify();
        if (temperature <= pChar2_value_int){
            digitalWrite(led1,HIGH);
            digitalWrite(led2,LOW);
        } else {
            digitalWrite(led2,HIGH);
            digitalWrite(led1,LOW);
        }

        Serial.print("temprature is :\t");
        Serial.println(temperature);

        IES.setTextSize(1.8);
        IES.setCursor(0,0);
        IES.print("Temperature is :");

        IES.setTextSize(2);
        IES.setCursor(0,20);
        IES.print(temperature);

        IES.setTextSize(2);
        IES.setCursor(0,35);
        IES.print("centigrade");
        IES.display();
        delay(500);
        IES.clearDisplay();
        delay(1000);
    }
    else{
      temperature = voltage * 3.3/4096 * (-53.561) + 149.97;
      if (temperature <= pChar2_value_int){
            digitalWrite(led1,HIGH);
            digitalWrite(led2,LOW);
        } else {
            digitalWrite(led2,HIGH);
            digitalWrite(led1,LOW);
        }

        Serial.print("temprature is :\t");
        Serial.println(temperature);

        IES.setTextSize(1.8);
        IES.setCursor(0,0);
        IES.print("Temperature is :");

        IES.setTextSize(2);
        IES.setCursor(0,20);
        IES.print(temperature);

        IES.setTextSize(2);
        IES.setCursor(0,35);
        IES.print("centigrade");
        IES.display();
        delay(500);
        IES.clearDisplay();
        delay(1000);
    }
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        Serial.println("IES Thermometer project , Fazlollah Pour , Nekoonam , Jafari");
        oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
}
