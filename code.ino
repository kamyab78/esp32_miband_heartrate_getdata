#include "BLEDevice.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "uuid.h"
#include "utilize.h"
static BLEUUID serviceUUIDTime("0000fee0-0000-1000-8000-00805f9b34fb");
static BLEUUID charUUIDTime("00002A2B-0000-1000-8000-00805F9B34FB");
static BLEUUID serviceUUIDName("00001800-0000-1000-8000-00805f9b34fb");
static BLEUUID charUUIDName("00002a00-0000-1000-8000-00805f9b34fb");
static BLEUUID serviceUUIDWalk("0000FEE0-0000-1000-8000-00805f9b34fb");
static BLEUUID charUUIDWalk("00000007-0000-3512-2118-0009af100700");
static BLEUUID serviceUUIDHeart("0000180d-0000-1000-8000-00805f9b34fb");
static BLEUUID charUUIDHeart("00002a37-0000-1000-8000-00805f9b34fb");

WiFiClient espClient;
PubSubClient client(espClient);
String result_name,result_time,result_heart,result_walk;
static boolean doConnect = false;
static boolean isFinish = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
BLERemoteCharacteristic  * pHRMMeasureCharacteristic;
BLERemoteDescriptor    * cccd_hrm;
static BLEAdvertisedDevice* myDevice;
enum dflag {
  idle = 0,

};
dflag         status = idle;

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
  }
};
static void notifyCallback_heartrate(BLERemoteCharacteristic* pHRMMeasureCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    status = idle;
  Serial.printf("Get Heart Rate: ");
  Serial.printf("%d\n", pData[1]);
  
  // v--- What do you want to do when the device gets the HRM data? ---v
  // To do:
  
  
  
  // ^--- What do you want to do when the device gets the HRM data? ---^
}
bool connectToServer() {
    Serial.println(F("Connecting to Bluetooth"));
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(F("Created Bluetooth client"));
    pClient->setClientCallbacks(new MyClientCallback());
    pClient->connect(myDevice);
    Serial.println("Connected to Bluetooth server");
    BLERemoteService* pRemoteService;
    ///////////////////////////////////////////////GET HEART /////////////////////////////////////////////////////////
    pRemoteService = pClient->getService(heart_rate_sev_uuid);
    pHRMMeasureCharacteristic = pRemoteService->getCharacteristic(UUID_CHAR_HRM_MEASURE);
    cccd_hrm = pHRMMeasureCharacteristic->getDescriptor(CCCD_UUID);
   pHRMMeasureCharacteristic->registerForNotify(notifyCallback_heartrate);
      cccd_hrm->writeValue(HRM_NOTIFICATION, 2, true);

     pRemoteService = pClient->getService(serviceUUIDHeart);
 if (pRemoteService == nullptr){
  Serial.println("HeartRate is null");
 }
 else{
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUIDHeart);
  if (pRemoteCharacteristic == nullptr) {
      Serial.println(F("Failed to find Heart characteristic UUID"));
      pClient->disconnect();
      return false;
    }
// if(pRemoteCharacteristic->canRead()) {
  std::string heartValue = pRemoteCharacteristic->readValue();
        Serial.println(F("The characteristic of Heart value is:"));
        result_heart= heartValue.c_str();
  Serial.println(heartValue.c_str());
//  }

 }
////////////////////////////////////////////////////GET Walk/////////////////////////////////////////////////////////
 pRemoteService = pClient->getService(serviceUUIDWalk);
 if (pRemoteService == nullptr){
      Serial.println(F("Failed to find Heart service UUID"));
 }
 else{
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUIDWalk);

  if (pRemoteCharacteristic == nullptr) {
      Serial.println(F("Failed to find Walk characteristic UUID"));
      pClient->disconnect();
      return false;
    }
   
 if(pRemoteCharacteristic->canRead()) {
        Serial.println(F("The characteristic of Step value is:"));
  std::string walkValue = pRemoteCharacteristic->readValue();
  result_walk= walkValue.c_str();
  Serial.println(walkValue.c_str());

  }
 }
     

     
    



////////////////////////////////////////////////////GEt TIME //////////////////////////////////////////////////////////////
    pRemoteService = pClient->getService(serviceUUIDTime);
    if (pRemoteService == nullptr) {
      Serial.println(F("Failed to find Time service UUID"));
      pClient->disconnect();
      return false;
    }
    Serial.println(F("Found Time service"));
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUIDTime);
    if (pRemoteCharacteristic == nullptr) {
      Serial.println(F("Failed to find Time characteristic UUID"));
      pClient->disconnect();
      return false;
    }
    Serial.println(F("Found Time characteristic"));
    if(pRemoteCharacteristic->canRead()) {
      pRemoteCharacteristic->readValue();

      uint8_t* timeValue = pRemoteCharacteristic->readRawData();
      Serial.print(F("The characteristic of Time value is: "));
      Serial.printf("%d : %d : %d \n",timeValue[4],timeValue[5],timeValue[6]);
      result_time=String((int)timeValue[4])+":"+String((int)timeValue[5])+":"+String((int)timeValue[6]);
    }
    else
      Serial.println("The Characteristic Time is now readable");
      
//////////////////////////////////////////////////////////GET NAME ////////////////////////////////////////////////////////
     pRemoteService = pClient->getService(serviceUUIDName);
    if (pRemoteService == nullptr) {
      Serial.println(F("Failed to find Name service UUID"));
      pClient->disconnect();
      return false;
    }
    Serial.println(F("Found Name Service"));
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUIDName);
    if (pRemoteCharacteristic == nullptr) {
      Serial.println(F("Failed to find Name characteristic UUID: "));
      pClient->disconnect();
      return false;
    }

    Serial.println(F("Found Name characteristic"));
    if(pRemoteCharacteristic->canRead()) {
      std::string nameValue = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic Name value was: ");
      Serial.println(nameValue.c_str());
      result_name=nameValue.c_str(); 
    }
    else
      Serial.println("The Characteristic Time is now readable");
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String finalres =result_name+" - "+result_time+" - "+result_heart+" - "+result_walk;
Serial.print(F("Ready to publish :"));
Serial.println(finalres.c_str());
client.publish("97522184/embedded",finalres.c_str());
Serial.println(F("Publish Done"));
    return true;
}

 BLEScan* pBLEScan;
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if(advertisedDevice.getName()!=""){
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());      
    }
      if(advertisedDevice.getAddress().toString()=="ca:3c:38:4a:23:7f"){
      Serial.println(F("MI Band 2 Found !!!"));
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
    }else{
      doConnect = false;
      } 
  } 
};

void setup() {
  Serial.println(F("Starting the Project"));
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  WiFi.begin("kamyab", "76777811");
  Serial.println(F("Wifi Start"));

  while(WiFi.status() != WL_CONNECTED){
    Serial.println(F("Connecting to WiFi Kamyab.."));
   delay(5000);
  }
  Serial.println(F("Wifi Connected"));
  Serial.println(F("Start Connection to MQTT Server"));
  client.setServer("45.149.77.235", 1883); 
  while (!client.connected()) {
    Serial.println(F("Connecting to MQTT..."));
    if (client.connect("ESP32Client", "97522184", "C1yqkzaN" )) {
       Serial.println("MQTT Server Connected");
    } else {
      Serial.print("failed with connection to MQTT Server ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  Serial.println("Starting Bluetooth");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
} 
void loop() {
  if(!isFinish){
      if (doConnect == true) {
        digitalWrite(2, HIGH);
        if(connectToServer())
          isFinish = true;
      }
  else{
      Serial.println(F("Target device not Found :( "));
      Serial.println("Searching for Bluetooth Devices");
      pBLEScan->start(1, false);
      delay(1000);
      digitalWrite(2, LOW);
  }
  delay(1000);
  }


}
