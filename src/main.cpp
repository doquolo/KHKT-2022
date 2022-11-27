#include <ArduinoJson.h>
#include <ESP32WebServer.h>
#include <WiFi.h>
#include <Arduino.h>
#include "LoRa_E32.h"
#include <bits/stdc++.h>
#include <HardwareSerial.h>

#include "index.h"

// setting up lora device
LoRa_E32 e32ttl100(&Serial2, 15, 19, 21);

// setting up serial
HardwareSerial myserial(1);

// 10sec interval
unsigned long msbefore = 0;

// ap credential
const char* ap = "device1";
const char* pass = "matkhau1";


ESP32WebServer server(80);

// "inbox"
std::vector<String> messagein;

// relaying
std::set<String> relayedmessage;

// duplication test
bool isDuplicated(String str) {
    std::set<String>::size_type before = relayedmessage.size();
    relayedmessage.insert(str);
    if (before == relayedmessage.size()) return false;
    else return true;
}

// "compress/decompress" message
DynamicJsonDocument decode(String str) {
  String name = "";
  String content = "";
  String type(str[str.length()-2]);
  String isEncrypted(str[str.length()-1]);
  bool part1end = false;
  for (int i = 0; i < (str.length())-3; i++) {
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
  doc["encrypted"] = isEncrypted;
  return doc;
}

// handle incoming message from lora module
int handleMessageIn() {
  ResponseContainer rc = e32ttl100.receiveMessage();
  // if sth goes wrong
  if (rc.status.code != 1) Serial.println(rc.status.getResponseDescription());
  else {
    // process message
    if (rc.data != "ping" || rc.data != "null|null|null") {
      // check whether the message has been received or not
      if (isDuplicated(rc.data)) {
        // continue like normal
        messagein.push_back(rc.data);
        Serial.println(rc.data);
        Serial.println(rc.status.getResponseDescription());
      }
      // if yes then... 
      else Serial.println("dupicated!");
    }
  }
  return 1;
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
    outgoingjson["e"] = (messjson["encrypted"].as<String>());
  }
  if (outgoingjson["t"] == "c") {
    // convert json to string
    String uwu;
    serializeJson(outgoingjson, uwu);
    // sent
    server.send(200, "application/json", uwu );
  }
}

// return homepage to client
void handleMain() {
  String s = mainpage;
  server.send(200, "text/html", s);
}

// handle control commands
void handleControl(String json) {
    DynamicJsonDocument command(1024);
    json.trim();
    Serial.print("Sending decoded command to device: ");
    deserializeJson(command, json);
    String commandstr = (command["c"].as<String>());
    Serial.println(commandstr);
    myserial.println(commandstr);
}

void setup() {
  // redefine serial 1
  myserial.begin(9600, SERIAL_8N1, 22, 23); // rx 22 tx 23
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
  // handle outgoing packet
  server.on("/update", handleUpdate);
  // handle decoded command
  server.on("/control", HTTP_POST, []() {
    handleControl(server.arg("plain"));
  });
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
  // handle interval
  if (millis() - msbefore >= 10000) {
    relayedmessage.clear();
    Serial.println("Cleared anti-repeated memory!");
    // update new "before point"
    msbefore = millis();
  }
  // idk
  delay(1);
}
