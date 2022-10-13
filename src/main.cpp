#include <ArduinoJson.h>
#include <ESP32WebServer.h>
#include <WiFi.h>
#include <Arduino.h>
#include "LoRa_E32.h"
#include <bits/stdc++.h>
#include <NMEAGPS.h>
#include <HardwareSerial.h>

#include "index.h"

// setting up lora device
LoRa_E32 e32ttl100(&Serial2, 15, 19, 21);

// setting up gps
HardwareSerial myserial(1);
NMEAGPS gps;

// device name
const String name = "device2";
// ap credential
const char* ap = "device1";
const char* pass = "zhoeuwu";

ESP32WebServer server(80);

// "inbox"
std::vector<String> messagein;


// GPS
// current gps info
DynamicJsonDocument currentGPSinfo(1024);
// handling gps request 
void handleGPS() {
  Serial.println("GPS request!");
  String uwu1;
  DynamicJsonDocument gpsinfo = currentGPSinfo;
  serializeJson(gpsinfo, uwu1);
  server.send(200, "application/json", uwu1);
}

// "compress/decompress" message
DynamicJsonDocument decode(String str) {
  String name = "";
  String content = "";
  String type(str[str.length()-1]);
  bool part1end = false;
  for (int i = 0; i < (str.length())-2; i++) {
    if (part1end) content += str[i];
    else {
      if (str[i] == 124) {
        part1end = true;
        continue;
      }
      name += str[i];
    }
  }  
  DynamicJsonDocument doc(1024);
  doc["name"] = name;
  doc["content"] = content;
  doc["type"] = type;
  return doc;
}

String encode(DynamicJsonDocument doc) {
  String name, content, type;
  name = doc["n"].as<String>();
  content = doc["m"].as<String>();
  type = doc["t"].as<String>();
  return (name + "|" + content + "|" + type);
}

// lora comm
void sendviaLora(String data) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, data);
  ResponseStatus rs = e32ttl100.sendMessage(encode(doc)); 
  Serial.print("Sent via LoRa message (status): ");
  Serial.print(data);
  Serial.println(rs.getResponseDescription());
}

// handle incoming message from lora module
void handleMessageIn() {
  ResponseContainer rc = e32ttl100.receiveMessage();
  // if sth goes wrong
  if (rc.status.code != 1) Serial.println(rc.status.getResponseDescription());
  else {
    // process message
    if (rc.data != "ping") {
      messagein.push_back(rc.data);
      Serial.println(rc.data);
      Serial.println(rc.status.getResponseDescription());
    }
  }
}

// sending message
void handleMessageOut(String json) {
    json.trim();
    Serial.print("Received message: ");
    Serial.println(json);
    sendviaLora(json);
}

// updating new message
void handleUpdate() {
  DynamicJsonDocument outgoingjson(1024);
  // process incoming message
  if (messagein.size() == 0) {
    // create outgoing json
    outgoingjson["m"] = "-1";
    outgoingjson["n"] = "esp32";
    outgoingjson["t"] = "-1";
  }
  if (messagein.size() != 0) {
    DynamicJsonDocument messjson = decode(messagein[0]);
    messagein.erase(messagein.begin());
    // create outgoing json
    outgoingjson["n"] = (messjson["name"].as<String>());
    outgoingjson["m"] = (messjson["content"].as<String>());
    outgoingjson["t"] = (messjson["type"].as<String>());
  }
  // convert json to string
  String uwu;
  serializeJson(outgoingjson, uwu);
  // sent
  server.send(200, "application/json", uwu );
}

// return homepage to client
void handleMain() {
  String s = mainpage;
  server.send(200, "text/html", s);
}

void setup() {
  // redefine serial 1
  myserial.begin(9600, SERIAL_8N1, 22, 23);
  //  WiFi setup (ap mode)
  Serial.print("Enabling SoftAP...");
  Serial.println(WiFi.softAP(ap, pass));
  //  begin serial
  Serial.begin(9600);
  delay(1000);
  Serial.print("SoftAp IP address = ");
  Serial.println(WiFi.softAPIP());
  // startup all pins and UART
  e32ttl100.begin();
  Serial.println("Test message: ping \n Status: ");
  // // send test message
  ResponseStatus rs = e32ttl100.sendMessage("ping");
  Serial.println(rs.getResponseDescription());
  // startup local sv
  server.on("/", handleMain);
  // handle incomming packet
  server.on("/send", HTTP_POST, []() {
      handleMessageOut(server.arg("plain"));
  });
  // handle outgoing packet
  server.on("/update", handleUpdate);
  // handle gps request
  server.on("/gps", handleGPS);
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  // handle connected clients
  server.handleClient();

  // handle incoming message from lora module
  if (e32ttl100.available()>1) {
    Serial.println("Incoming lora message!");
    handleMessageIn();
  } 
  if (gps.available(myserial)) {
    gps_fix fix = gps.read();
    currentGPSinfo["lat"] = fix.latitude();
    currentGPSinfo["long"] = fix.longitude();
    currentGPSinfo["sat"] = fix.satellites;
    currentGPSinfo["time"] = fix.dateTime_ms();
    Serial.print(fix.status);
    Serial.print("-");
    Serial.print(fix.latitude());
    Serial.print("-");
    Serial.print(fix.longitude());
    Serial.print("-");
    Serial.print(fix.satellites);
    Serial.print("-");
    Serial.println(fix.dateTime_ms());
  }
  // idk
  delay(1);
}