
/*
 * 
 * This detects advertising messages of BLE devices and compares it with stored MAC addresses. 
 * If one matches, it sends an MQTT message to swithc something

   Copyright <2017> <Andreas Spiess>
   Based on Neil Kolban's example file: https://github.com/nkolban/ESP32_BLE_Arduino
 
 
 */
#include <ArduinoJson.h>
#include "BLEDevice.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

static BLEAddress *pServerAddress;


#define PIR 27
#define LEDPIR 26
#define LEDWF 2
#define LEDBT 15


BLEScan* pBLEScan;
BLEClient*  pClient;

String ble_address;
String ble_name;
String ble_data;
String valueof_rssi;
bool deviceFound = false;

String knownAddresses[] = {"48:e7:29:9e:8e:ae", "48:e7:29:96:7a:8a", "0c:b8:15:f7:2a:82", "0a:13:42:e5:ba:0b", "08:d1:f9:34:f3:12", "08:d1:f9:35:c1:5e", "08:d1:f9:34:f5:0e", "a0:a3:b3:28:39:5e"};

//const char* ssid = "Engineering24";
//const char* password = "12345678";
const char* ssid = "Riririrririri"; //SiunyilSMP2.4G ATAU unjkerja
const char* password = "999999999"; //unyilmintaduit3000 ATAU rakartini123

//const char* mqtt_server = "test.mosquitto.org";  // change for your own MQTT broker address
//#define TOPIC "ESP32/BLEDoorAccess"  // Change for your own topic
//#define PAYLOAD "1"    // change for your own payload

const char* mqtt_server = "broker.emqx.io";//server address
const int port=1883;//server port
const char* user          = "Yusri";
const char* pass          = "Esp@32";
const char* client_id     = "mqttx_4e57a5ec";
//const char* topicResult   = "ESP32BLE/DoorAccess";
const char* topicSubs     = "ESP32BLE/ATTENDANCESYSTEM"; //"esp32door/#" "ESP32BLE/DoorAccess"

unsigned long entry;

WiFiClient espClient;
PubSubClient MQTTclient(espClient);

/*******************************************************************
 BLE IS WORKING
 
 *******************************************************************/

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());
      //ble_data=advertisedDevice.toString().c_str();
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      ble_data = pServerAddress->toString().c_str();


      bool known = false;
      for (int i = 0; i < (sizeof(knownAddresses) / sizeof(knownAddresses[0])); i++) {
        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[i].c_str()) == 0) known = true;
      }
      if (known) {
        Serial.print("Device found: ");
        Serial.println(advertisedDevice.getRSSI());
        valueof_rssi=advertisedDevice.getRSSI();       
        if (advertisedDevice.getRSSI() > -83) deviceFound = true;
        else deviceFound = false;
        Serial.println(pServerAddress->toString().c_str());
        ble_address=pServerAddress->toString().c_str(); ///display address of BLE ID / SERVER
        Serial.println(advertisedDevice.toString().c_str());
        //ble_data = advertisedDevice.toString().c_str(); ///display packet of advertising data (name, address, service uuid, power tx)        
        advertisedDevice.getScan()->stop();
      }

      /******** This releases the memory when we're done. ********/
      delete pServerAddress; 
      digitalWrite(LEDWF, HIGH);
      digitalWrite(LEDBT, LOW);
      
   }
}; // MyAdvertisedDeviceCallbacks

/*******************************************************************
 WIFI IS WORKING
 
 *******************************************************************/

void sendMessage() {
  //btStop();
  StaticJsonDocument<500> data;
  char sentSrvr[512];
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  entry = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - entry >= 15000) esp_restart();
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
  MQTTclient.setServer(mqtt_server, 1883);
  MQTTclient.setCallback(MQTTcallback);
  Serial.println("Connect to MQTT server...");
  while (!MQTTclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (MQTTclient.connect(client_id, user, pass)) {
      Serial.println("connected");
      data["deviceFound"]=ble_address;
      serializeJsonPretty(data,sentSrvr);           
      MQTTclient.publish("ESP32BLE/ATTENDANCESYSTEM", sentSrvr); 
      serializeJsonPretty(data,Serial);
//      lcd.setCursor(1,1);         
//      Serial.println("CONNECTING TO WIFI");
//      lcd.clear();
       
    } else {
      Serial.print("failed, rc=");
      Serial.print(MQTTclient.state());
      Serial.println(" try again in 5 seconds");
 //     lcd.setCursor(0,1);         
 //     Serial.println("FAIL TO CONNECT WIFI");
//      lcd.clear();
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
  for (int i = 0; i > 10; i++) {
    MQTTclient.loop();
    delay(100);
  }
  MQTTclient.disconnect();
  delay(100);
  WiFi.mode(WIFI_OFF);
  btStart();
  digitalWrite(LEDWF, LOW);
  digitalWrite(LEDBT, HIGH);
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  pinMode(LEDPIR, OUTPUT);
  pinMode(LEDWF, OUTPUT);
  pinMode(LEDBT, OUTPUT);
  pinMode(PIR, INPUT);
  lcd.init();     // initialize LCD                 
  lcd.backlight();  // turn on Backlight
  //pinMode(PIR, INPUT);
  lcd.setCursor(5, 1);
  lcd.print("WELCOME TO");
  lcd.setCursor(4, 2);
  lcd.print("AL'S PROJECT");
  delay(3000);
  lcd.clear();
  
  
  BLEDevice::init("");

  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
}

void loop() {

  Serial.println();
  Serial.println("BLE Scan restarted.....");
  
  deviceFound = false;
  BLEScanResults scanResults = pBLEScan->start(30);

  int sensValue = digitalRead(PIR);
  //int Push_button_state = digitalRead(PushButton); 
  if (deviceFound && sensValue) {
/////////////////////////////////////////////////////////////////////////////////////x
    Serial.println("MOTION DETECTED & KNOWN DEVICE!"); 
    digitalWrite(LEDPIR, HIGH);
    lcd.setCursor(4, 0);
    lcd.print("GOT ADDRESS: ");
    lcd.setCursor(2, 1);
    lcd.print("");
    lcd.print(ble_data);
    lcd.setCursor(6, 2);
    lcd.print("RSSI: ");
    lcd.setCursor(12, 2);
    lcd.print("");
    lcd.print(valueof_rssi);
    
    sendMessage();
    Serial.println("Waiting for 15 seconds");
    delay(15000);
    

 }else if (deviceFound) {
    digitalWrite(LEDPIR, LOW);
    lcd.setCursor(4, 0);
    lcd.print("GOT ADDRESS: ");
    lcd.setCursor(2, 1);
    lcd.print("");
    lcd.print(ble_data);
    lcd.setCursor(6, 2);
    lcd.print("RSSI: ");
    lcd.setCursor(12, 2);
    lcd.print("");
    lcd.print(valueof_rssi);
    
 }else {
 
    digitalWrite(LEDPIR, LOW);
    Serial.println("NO MOTION");
    lcd.setCursor(6,0);
    lcd.print("NO MOTION");
    lcd.setCursor(6,1);
    lcd.print("DETECTED");
    lcd.setCursor(9,2);
    lcd.print("OR");
    lcd.setCursor(3,3);
    lcd.print("UNKNOWN ADDRESS");
    delay(6000);
    lcd.clear();
    
    }
} // End of loop
