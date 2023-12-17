#include <stdio.h>
#include <Arduino.h>

#include "LocoNetSerial.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>

LocoNetBus bus;

#define LOCONET_PIN_RX 16
#define LOCONET_PIN_TX 17

#include <LocoNetESP32.h>
LocoNetESP32 locoNetPhy(&bus, LOCONET_PIN_RX, LOCONET_PIN_TX, 0);
LocoNetDispatcher parser(&bus);

unsigned long previousMillis = millis(); 

bool State = LOW;             //Feedback state

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
    
    WiFiManager wifiManager;
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
    Serial.println(WiFi.localIP());
}


void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 2000) {
    previousMillis = currentMillis;
    if (State == LOW) {
      State = HIGH;
    } else {
      State = LOW;
    }
    // send the Feedback with the State of the variable:
    Serial.print(State);
    Serial.print(" change:_");

    sendSensor(41,State);
    reportSensor(&bus, 22, State);
   
  }
}

// Rückmeldesonsor Daten senden
void sendSensor(int Adr, boolean state) {
  //Adressen von 1 bis 4096 akzeptieren
  if (Adr <= 0)        //nur korrekte Adressen
    return;
  Adr = Adr - 1;    
  byte D2 = Adr >> 1;    //Adresse Teil 1 erstellen
  bitWrite(D2,7,0);        //A1 bis A7
    
  byte D3 = Adr >> 8;     //Adresse Teil 2 erstellen, A8 bis A11
  bitWrite(D3,4, state);    //Zustand ausgeben
  bitWrite(D3,5,bitRead(Adr,0));    //Adr Bit0 = A0
    

  //Checksum bestimmen:  
  byte D4 = 0xFF;        //Invertierung setzten
  D4 = OPC_INPUT_REP ^ D2 ^ D3 ^ D4;  //XOR
  bitWrite(D4,7,0);     //MSB Null setzten


  LnMsg SendPacket;
  
  SendPacket.data[0] = OPC_INPUT_REP; //0xB2
  SendPacket.data[1] = D2;    //1. Daten Byte IN1
  SendPacket.data[2] = D3;    //2. Daten Byte IN2
 
  sendMsg(&bus,SendPacket);

 
}
