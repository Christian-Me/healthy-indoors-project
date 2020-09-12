#define MODULE_NAME "espnow2monitor"
#include <Arduino.h>
#include "../../include/datatypes.h"

// struct_message incomingReadings;
dataPacket packet;
// data collection and storage
#include "datacollection.h"
dataCollection dataStorage;

// communication plugin
#ifdef COMUNICATION_ESPNOW
  #include "c_espnow.h"
#endif

// module specific libraries
#ifdef MONITOR_NEOPIXEL
  #include "neopixel.h"
#endif
#ifdef MONITOR_EPAPER
  #include "epaper.h"
  #include "chart.h"
  #include "ui.h"
#endif

// handle incoming data
void dataReceived(uint8_t *mac_addr, uint8_t *data, uint8_t dataLength) {
  digitalWrite(LED_BUILTIN, LOW);
  // Copies the sender mac address to a string
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  memcpy(&packet, data, sizeof(packet));
  Serial.printf("Sensor accuracy     : %d \n", packet.accuracy);
  uint8_t index = dataStorage.update(macStr, &packet);

  #ifdef MONITOR_EPAPER
    m_chart_addSample("iaq");
  #endif

  digitalWrite(LED_BUILTIN, HIGH);
}
 
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);     // initialize serial port

  Serial.println();
  Serial.println(F("-----------------------------------------------"));
  Serial.println(F("Healthy indoor project"));
  #ifdef MONITOR_NEOPIXEL
    Serial.println(F("espnow2neopixel"));
  #endif
  #ifdef MONITOR_EPAPER
    Serial.println(F("espnow2epaper"));
  #endif
  Serial.println(F("Version 0.0.1"));
  Serial.println(F("-----------------------------------------------"));
  Serial.println();
  dataStorage.init();

  #ifdef MONITOR_EPAPER // init e-paper first for splash screen
    m_epaper_setup();
  #endif

  #ifdef COMUNICATION_ESPNOW // using esp-now
    // 1. register callback for receiveing data
    c_espnow_registerReceiveCB(&dataReceived);
    // 2. initialize espnow
    c_espnow_init("espnow2epaper");
  #endif
  #ifdef MONITOR_NEOPIXEL // using neopixel and setting the brightness value
    m_neopixel_init(255);
  #endif
  #ifdef MONITOR_EPAPER // using epaper
    m_chart_init();
    m_ui_init();
  #endif
  Serial.println(F("Done."));
  Serial.println();

}

void loop() {
  #ifdef COMUNICATION_ESPNOW  // communicator loop
    c_espnow_loop();
  #endif
  #ifdef MONITOR_NEOPIXEL // neopixel loop
    m_neopixel_loop();
  #endif
  #ifdef MONITOR_EPAPER // epaper loop
    m_ui_loop();    // refresh ui
    m_chart_loop(); // refresh chart
  #endif
}
