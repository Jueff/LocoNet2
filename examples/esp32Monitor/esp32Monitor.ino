#include <stdio.h>
#include <Arduino.h>

#include "LocoNetSerial.h"

//#include <WiFi.h>
//#include <ESPmDNS.h>
//#include <WiFiManager.h>

LocoNetBus bus;

#define LOCONET_PIN_RX 4
#define LOCONET_PIN_TX 13

#include <LocoNetESP32.h>
LocoNetESP32 locoNetPhy(&bus, LOCONET_PIN_RX, LOCONET_PIN_TX, 3);
LocoNetDispatcher parser(&bus);


void setup() {

    Serial.begin(115200);
    Serial.println("Ultimate LocoNet Command Station");

   locoNetPhy.begin();
      
    
    parser.onPacket(CALLBACK_FOR_ALL_OPCODES, [](const lnMsg *rxPacket) {
        
        char tmp[100];
        formatMsg(*rxPacket, tmp, sizeof(tmp));
        Serial.printf("onPacket: %s\n", tmp);

    });


    parser.onSwitchRequest([](uint16_t address, bool output, bool direction) {
        Serial.print("Switch Request: ");
        Serial.print(address, DEC);
        Serial.print(':');
        Serial.print(direction ? "Closed" : "Thrown");
        Serial.print(" - ");
        Serial.println(output ? "On" : "Off");
    });
    parser.onSwitchReport([](uint16_t address, bool state, bool sensor) {
        Serial.print("Switch/Sensor Report: ");
        Serial.print(address, DEC);
        Serial.print(':');
        Serial.print(sensor ? "Switch" : "Aux");
        Serial.print(" - ");
        Serial.println(state ? "Active" : "Inactive");
    });
    parser.onSensorChange([](uint16_t address, bool state) {
        Serial.print("Sensor: ");
        Serial.print(address, DEC);
        Serial.print(" - ");
        Serial.println(state ? "Active" : "Inactive");
    });
    
/*    WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(300); // 5 min
  if ( !wifiManager.autoConnect("ESP32CommandStation AP") ) {
    delay(1000);
        Serial.print("Failed connection");
    ESP.restart();
  }
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());*/
    
}


void loop() {


}
