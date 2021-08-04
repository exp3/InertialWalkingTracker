#include <M5StickC.h>
// Bluetooth LE
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

///////////////
bool updateColor;
String lastColor;

float pitchG = 0;
float rollG = 0;
float yawG = 0;
float x = 0;
float y = 0;
float z = 0;
float pitchA = 0;
float rollA = 0;
float yawA = 0;

int intPitchG = 0;
int intRollG = 0;
int intYawG = 0;
int intx = 0;
int inty = 0;
int intz = 0;
int intPitchA = 0;
int intRollA = 0;
int intYawA = 0;

String stringValue;

std::string ToString(int);

// the setup routine runs once when M5Stack starts up
void setup() {
  // Initialize the M5Stack object
  M5.begin();

  M5.Lcd.print("Setup....");

  initBLE();
  //initLCDcolor();
  M5.IMU.Init();

  M5.Lcd.println("Done");
  M5.Lcd.fillScreen(BLACK);
}

// the loop routine runs over and over again forever
void loop() {
  M5.update();
  loopBLE();
  loopSensor();
}

///////////////////
// Bluetooth LE  //
///////////////////
BLEServer *pServer = NULL;
BLECharacteristic *pNotifyCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define LOCAL_NAME "M5Stack-Color"
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "e5a1c9a8-ab93-11e8-98d0-529269fb1459"
#define CHARACTERISTIC_UUID_RX "e5a1cda4-ab93-11e8-98d0-529269fb1459"
#define CHARACTERISTIC_UUID_NOTIFY "e5a1d146-ab93-11e8-98d0-529269fb1459"

// Bluetooth LE Change Connect State
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) { deviceConnected = true; };

  void onDisconnect(BLEServer *pServer) { deviceConnected = false; }
};

// Bluetooth LE initialize
void initBLE() {
  // Create the BLE Device
  BLEDevice::init(LOCAL_NAME);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pNotifyCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_NOTIFY, BLECharacteristic::PROPERTY_NOTIFY);

  pNotifyCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
}

// Bluetooth LE loop
void loopBLE() {
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("startAdvertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

void loopSensor() {
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("ahrs\n");
  M5.IMU.getAhrsData(&pitchG, &rollG, &yawG);
  M5.Lcd.printf("p= %7.2f\n", pitchG);
  M5.Lcd.printf("r= %7.2f\n", rollG);
  M5.Lcd.printf("y= %7.2f\n", yawG);

  M5.Lcd.printf("accel\n");
  M5.IMU.getAccelData(&x, &y, &z);
  M5.Lcd.printf("y= %7.2f\n", x);
  M5.Lcd.printf("x= %7.2f\n", y);
  M5.Lcd.printf("z= %7.2f\n", z);

  M5.Lcd.printf("gyro\n");
  M5.IMU.getGyroData(&pitchA, &rollA, &yawA);
  M5.Lcd.printf("p= %7.2f\n", pitchA);
  M5.Lcd.printf("r= %7.2f\n", rollA);
  M5.Lcd.printf("y= %7.2f\n", yawA);

  stringValue = "\n";
  stringValue += String((int)(pitchG * 100)) + ",";
  stringValue += String((int)(rollG * 100)) + ",";
  stringValue += String((int)(yawG * 100)) + ",";
  stringValue += String((int)(x * 100)) + ",";
  stringValue += String((int)(y * 100)) + ",";
  stringValue += String((int)(z * 100)) + ",";
  stringValue += String((int)(pitchA * 100)) + ",";
  stringValue += String((int)(rollA * 100)) + ",";
  stringValue += String((int)(yawA * 100));

  if (deviceConnected) {
    char sendMessage[150];
    stringValue.toCharArray(sendMessage, 150);
    pNotifyCharacteristic->setValue(sendMessage);
    pNotifyCharacteristic->notify();
  }
  delay(10);
}