#define MODULE_NAME "espnow2monitor"
#include <Arduino.h>
#include "../../include/datatypes.h"
#include "utils.h"

// struct_message incomingReadings;
dataPacket packet;

// communication plugin
#ifdef COMUNICATION_ESPNOW
  #include "c_espnow.h"
  c_espnow comm1;
#endif

// handle incoming data

void dataReceived(const uint8_t *mac_addr, const uint8_t *data, int dataLength) {

  digitalWrite(LED_BUILTIN, LOW);
  printDataPackage(data,dataLength);

  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  memcpy(&packet, data, sizeof(packet));
  digitalWrite(LED_BUILTIN, HIGH);
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);     // initialize serial port

  Serial.println();
  Serial.println(F("-----------------------------------------------"));
  Serial.println(F("Healthy indoor project"));
  Serial.println(F("Node: Monitor"));
  #ifdef MONITOR_NEOPIXEL
    Serial.println(F("Display: neopixel"));
  #endif
  #ifdef MONITOR_EPAPER
    Serial.println(F("Display: epaper"));
  #endif
  #ifdef MONITOR_TFT
    Serial.println(F("Display: TFT"));
  #endif
  Serial.println(F("Version 0.0.1"));
  Serial.println(F("-----------------------------------------------"));
  Serial.println();
  
  #ifdef COMUNICATION_ESPNOW // using esp-now
    // 1. register callback for receiveing data
    comm1.registerReceiveCallback(&dataReceived);
    // 2. initialize espnow
    comm1.init("espnow2epaper");
  #endif
  Serial.println(F("Done."));
  Serial.println();

}

void loop() {
  #ifdef COMUNICATION_ESPNOW  // communicator loop
    comm1.loop();
  #endif
}
