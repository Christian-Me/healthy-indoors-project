/*
  Healthy Indoors Project

  basic sensor node

  DESCRIPTION:
  the data from tis sensor node is sent into esp-now local network
  no access point is required

  LIMITATIONS:
   - deep sleep is disabled until the calibration works
   - currently only tested with the BME680
  
  IMPROVEMENTS
   - avoid restart of the calibration process after wake up from deep sleep (BME680)
     example here: https://github.com/BoschSensortec/BSEC-Arduino-library/tree/master/examples/esp32DeepSleep

  This project was started September 2020 by Christian Meinert
  as a reaction of the global COVID-19 epidemic and the need of an as healthy as possible indoor enviornment
  as it became obvious that the virus is likely to be able to spread via aerosols which could stay longer
  in the air and travel further inside rooms as droplets.

  original project location on Github :

  Stay healthy, keep distance, open windows!
*/

#include <Arduino.h>
#include "config.h"
#include <ESP8266WiFi.h>
#include <espnow.h>
#include "../../include/datatypes.h"
#include "BME680.h"

// module specific libraries
#ifdef MONITOR_NEOPIXEL
  #include "neopixel.h"
#endif

#define NODE_NAME         "Health room monitor NODE"
#define NODE_ROLE         ESP_NOW_ROLE_CONTROLLER         // set the role of this device: CONTROLLER, SLAVE, COMBO or MAX
#define RECEIVER_ROLE     ESP_NOW_ROLE_SLAVE              // set the role of the receiver
#define WIFI_CHANNEL      6

// Deep sleep mode
// to enable deep sleep you have to connect RST to D0/GPIO16 to wake up out of deep sleep
// to flash a device in deep sleep connect GND to D3/GPIO0 and press reset to go into flash mode. Remove connection after sucsessfull upload

// #define DEEP_SLEEP                                   // uncomment for deep sleep (Connect RST to D0/GPIO16 to wake up)
#define SLEEP_SEC       10                              // sleep time (sec)
#define SECONDS_TO_US   1000000

// sensors
BME680 sensor1;

uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};     // please update this with the MAC address of the receiver if you do not prefere bradcasts (i.e. mutiple rooms)

void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
  
  if(transmissionStatus == 0) {
    Serial.println(F("Data sent successfully"));
  } else {
    Serial.print(F("Error code: "));
    Serial.println(transmissionStatus);
  }
  
  digitalWrite(LED_BUILTIN, HIGH);
}

dataPacket packet;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);     // initialize serial port
  Serial.println();
  Serial.println(F("-----------------------------------------------"));
  Serial.println(F("Healthy indoors project"));
  Serial.println(F("sensor node"));
  Serial.println(F("version 0.0.1"));
  Serial.println(F("-----------------------------------------------"));

  Serial.println(F("Initializing Sensor"));
  sensor1.sensorInit();
  Serial.println();
  Serial.print(F("Initializing Network."));
  Serial.println();


  Serial.println(F("ESP Now - peer to peer over wifi"));
  Serial.println(F("-----------------------------------------------"));

  #ifdef MONITOR_NEOPIXEL // using neopixel and setting the brightness value
    m_neopixel_init(255);
  #endif

  Serial.print(F("Node Name: "));
  Serial.println(NODE_NAME);
  Serial.print(F("MAC address: "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("WIFI channel: "));
  Serial.println(WIFI_CHANNEL);
  char macStr[18];
  Serial.print(F("Receiver MAC: "));
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           receiverAddress[0], receiverAddress[1], receiverAddress[2], receiverAddress[3], receiverAddress[4], receiverAddress[5]);
  Serial.println(macStr);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();        // we do not want to connect to a WiFi network
  
  if(esp_now_init() != 0) {
    Serial.println(F("ESP-NOW initialization failed"));
    return;
  }

  esp_now_set_self_role(NODE_ROLE);   
  esp_now_register_send_cb(transmissionComplete);   // this function will get called once all data is sent
  esp_now_add_peer(receiverAddress, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);

  Serial.println(F("ESP-NOW Initialized."));
  Serial.println();
}

void loop() {
  // red sensor data and post it to server (multiple sensors can be done in sequence)
  if (sensor1.sensorRead(&packet)) {
    digitalWrite(LED_BUILTIN, LOW);
    m_neopixel_showValue(0,packet.iaq,0,500);
    packet.battery=90;     // ToDo: replace by real battery levels
    packet.sensorIndex=0;  // ToDo: multiple sensors of the same type (if any use)
    esp_now_send(receiverAddress, (uint8_t *) &packet, sizeof(packet));
  } else {
    Serial.println(F("No new update available"));
  }
  #ifdef DEEP_SLEEP
    Serial.print(F("will go to sleep for "));
    Serial.print(SLEEP_SEC);
    Serial.println(" sec");

    ESP.deepSleep(SLEEP_SEC * SECONDS_TO_US);
  #endif
  #ifndef DEEP_SLEEP
    delay(3000); // delay is good for now. ToDo: Must be changed if sensor also receives data (to show maximum of all sensors in network)
  #endif
}