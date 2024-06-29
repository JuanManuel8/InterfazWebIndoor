/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp8266-nodemcu-websocket-server-sensor/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>
//#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include "DHT.h" // inclusión de librerías para temperatura por one wire
#include <IRsend.h>


#define DHTFTYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHTFPIN 4     // Pin digital connectado a sensor DHT para Floración


// Replace with your network credentials
const char* ssid = "tu red";
const char* password = "tu pass";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");
/////////////////////////////////////
// Set LED GPIO
//const int ledPin1 = 14;
//const int ledPin2 = 12;
//const int ledPin3 = 13;
const uint16_t kIrLed = 14;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

bool ledState = 0;
const int ledPin = 2;

String message = "";
String sliderValue1 = "22";
String sliderValue2 = "22";
String sliderValue3 = "22";

String state1="0";
String state2="0";
String state3="0";
String state4="0";
String state5="0";
String state6="0";


String Readings = "0";

int dutyCycle1;
int dutyCycle2;
int dutyCycle3;

//AC1 
//Mesg Desc.: Power: On, Mode: 2 (Auto), Fan: 0 (Auto0), Temp: 24C, Zone Follow: Off, Sensor Temp: Off
uint16_t rawDataON1[199] = {4434, 4272,  628, 1506,  630, 468,  626, 1512,  626, 1510,  628, 
468,  626, 468,  628, 1512,  624, 468,  628, 460,  632, 1510,  628, 470,  624, 466,  624, 
1512,  626, 1510,  628, 468,  626, 1504,  630, 470,  626, 470,  626, 466,  628, 1510,  626, 
1504,  636, 1508,  628, 1508,  626, 1508,  628, 1504,  630, 1510,  628, 1510,  626, 468,  
626, 466,  626, 468,  626, 470,  622, 470,  626, 460,  632, 1510,  628, 468,  626, 468,  
626, 1510,  626, 468,  626, 468,  626, 468,  628, 1508,  626, 466,  628, 1508,  628, 1502,  
636, 466,  628, 1508,  628, 1504,  632, 1510,  628, 5126,  4442, 4272,  628, 1508,  628, 466,  
630, 1508,  628, 1508,  626, 468,  628, 468,  624, 1512,  628, 468,  624, 468,  626, 1504,  
632, 466,  626, 462,  632, 1512,  624, 1510,  628, 466,  626, 1512,  626, 468,  624, 466,  
628, 468,  626, 1512,  624, 1510,  626, 1510,  628, 1512,  626, 1508,  628, 1510,  628, 1508,  
630, 1506,  628, 470,  626, 468,  626, 470,  626, 466,  624, 468,  626, 466,  626, 1510,  626, 
466,  628, 466,  628, 1502,  632, 468,  626, 466,  630, 464,  628, 1508,  628, 468,  626, 1510, 
628, 1506,  630, 460,  634, 1508,  628, 1508,  626, 1512,  626};

//Mesg Desc.: Power: Off
uint16_t rawDataOFF1[199] = {4438, 4272,  630, 1508,  628, 468,  626, 1502,  634, 1506,  632, 466,
626, 468,  626, 1508,  628, 468,  626, 468,  626, 1510,  626, 466,  626, 468,  628, 1504,  634, 
1510,  626, 468,  624, 1512,  624, 468,  628, 1508,  630, 1508,  626, 1508,  628, 1510,  626, 460,
634, 1508,  628, 1508,  630, 1508,  628, 464,  630, 464,  628, 468,  628, 466,  626, 1510,  628, 
466,  628, 466,  626, 1508,  628, 1508,  628, 1510,  628, 466,  624, 468,  626, 468,  628, 464,  
630, 464,  628, 466,  628, 468,  628, 464,  628, 1504,  632, 1510,  628, 1510,  626, 1510,  628, 
1506,  628, 5132,  4436, 4270,  630, 1510,  626, 468,  626, 1508,  630, 1510,  626, 468,  628, 
468,  626, 1508,  628, 468,  628, 466,  626, 1508,  628, 468,  628, 466,  628, 1508,  626, 1510,
628, 466,  628, 1508,  628, 466,  628, 1508,  626, 1508,  630, 1506,  630, 1508,  626, 468,  626,
1512,  628, 1508,  626, 1510,  630, 466,  628, 466,  628, 468,  626, 468,  626, 1510,  626, 468, 
626, 466,  628, 1506,  630, 1510,  626, 1508,  630, 464,  628, 466,  630, 466,  626, 468,  628, 
468,  626, 466,  628, 464,  628, 466,  628, 1508,  630, 1506,  628, 1510,  628, 1508,  628, 1508,
628};


// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 1000;


//Json Variable to Hold Values
JSONVar Values;
//////////////////////////////////////

// Set the GPIO to be used to sending the message.
IRsend irsend(kIrLed);  
// 

// Create a sensor object
//Adafruit_BME280 bme;         // BME280 connect to ESP32 I2C (GPIO 21 = SDA, GPIO 22 = SCL)

DHT dhtF(DHTFPIN, DHTFTYPE);

//float TF; // Temperatura ambiente floración
//float HF; // Humedad ambiente floración

// Init BME280
void initBME(){
    // inicialización del sensor, lo debe resolver la librería
   dhtF.begin();
   /*
  if (!dhtF.begin()) {
    Serial.println("Could not find a valid DHT sensor, check wiring!");
    while (1);
  }*/
}
//////////////////////////////////////////////
//Get Values
String getValues(){
  Values["sliderValue1"] = String(sliderValue1); // aire 1 temp
  Values["sliderValue2"] = String(sliderValue2); // aire 2 temp
  Values["sliderValue3"] = String(sliderValue3); // aire 3 temp
   
  Values["temperature"] = String(dhtF.readTemperature());
  Values["humidity"] =  String(dhtF.readHumidity());
  
  Values["state1"] = String(state1); // aire 1 on/off
  Values["state2"] = String(state2); // aire 1 modo
  Values["state3"] = String(state3); // aire 2 on/off
  Values["state4"] = String(state4); // aire 2 modo
  Values["state5"] = String(state5); // aire 3 on/off
  Values["state6"] = String(state6); // aire 3 modo
  
  String jsonString = JSON.stringify(Values);
  return jsonString;
}
///////////////////////////////////////////////
// 

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
   Serial.println("LittleFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
///////////////////////////////////////////
void notifyClientsO(String Values) {
  ws.textAll(Values);
  }
//void notifyClients() {
//  ws.textAll(String(ledState));//VER
//}

////////////////////////////////////////////


void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    /*String*/ message = (char*)data;
    // Check if the message is "getReadings"
    
    if (strcmp((char*)data, "getValues") == 0) {
      //if it is, send current sensor readings
      Readings = getValues();
      Serial.println(Readings);
      notifyClientsO(Readings);
    }
    
    yield();
    ////////////////////////////////////////////////////////////
    //AC1
   
    if (message.indexOf("1t") >= 0) {
      ledState = !ledState;
      notifyClientsO(getValues());
    }
    
    if (message.indexOf("1s") >= 0) {
      sliderValue1 = message.substring(2);
    
      if (sliderValue1 =="0"){
        irsend.sendRaw(rawDataOFF1, 199, 38);  // Send: vector raw data, estenión del vector, 38kHz.
        }else{
              irsend.sendRaw(rawDataON1, 199, 38);  // Send: vector raw data, estenión del vector, 38kHz.
              }  
      Serial.println(getValues());
      notifyClientsO(getValues());
    }
    //AC2
    if (message.indexOf("2s") >= 0) {
      sliderValue2 = message.substring(2);
      if (sliderValue2 =="0"){
        irsend.sendRaw(rawDataOFF1, 199, 38);  // Send: vector raw data, estenión del vector, 38kHz.
        }else{
              irsend.sendRaw(rawDataON1, 199, 38);  // Send: vector raw data, estenión del vector, 38kHz.
              }
      Serial.println(getValues());
      notifyClientsO(getValues());
    }
    //AC3    
    if (message.indexOf("3s") >= 0) {
      sliderValue3 = message.substring(2);
      if (sliderValue3 =="0"){
        irsend.sendRaw(rawDataOFF1, 199, 38);  // Send: vector raw data, estenión del vector, 38kHz.
        }else{
              irsend.sendRaw(rawDataON1, 199, 38);  // Send: vector raw data, estenión del vector, 38kHz.
              }
      Serial.println(getValues());
      notifyClientsO(getValues());
    }
   
    /*if (strcmp((char*)data, "getValues") == 0) {
      notifyClientsO(getValues());
    }*/
    ////////////////////////////////////////////////////////////
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void setup() {
  pinMode(4, INPUT); // D2 DTH
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  /////////////////////////////
  //pinMode(ledPin1, OUTPUT);
  //pinMode(ledPin2, OUTPUT);
  pinMode(kIrLed, OUTPUT);
  ////////////////////////////
  Serial.begin(115200);
  initBME();
  initWiFi();
  initFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
   });

  server.serveStatic("/", LittleFS, "/");

  // Start server
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    
    Readings = getValues();
    Serial.println(Readings);
    notifyClientsO(Readings);

  lastTime = millis();
  yield();
  }
  //////////////////////////////////////////////
  //analogWrite(ledPin1, dutyCycle1);
  //analogWrite(ledPin2, dutyCycle2);
  //analogWrite(ledPin3, dutyCycle3);
  /////////////////////////////////////////////
  digitalWrite(ledPin, ledState);
  ws.cleanupClients();
}
