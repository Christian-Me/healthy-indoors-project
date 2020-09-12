/*
  Healthy Indoors Project

  serial 2 HTTP node

  DESCRIPTION:
  the data from the esp-now local network received via the espnow2serial node is decoded and presented on a webpage hosted on the esp

  LIMITATIONS:
   - the ssid and password is defined by 
   - currently only the last reading from any sensor node is displayed
  
  IMPROVEMENTS
   - dynamically configure ssid and password
   - server the webpage via soft AP
   - display of dynamic amount of sensor nodes
   - sending alerts or notifications
   - plot values over time

  This project was started September 2020 by Christian Meinert
  as a reaction of the global COVID-19 epidemic and the need of an as healthy as possible indoor envioment
  as it became obvious that the virus is likely to be able to spread via aerosols which could stay longer
  in the air and travel further inside rooms as droplets.

  original project location on Github :

  Stay healthy, keep distance, open windows!
*/

// Replace with your network credentials of your router
const char* ssid = "LOL-AP-01";
const char* password = "WLAN1lrB&bB2!AP1";

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include "../../include/datatypes.h"
#include <SoftwareSerial.h>
#include <CRC32.h>

#include <webserver.h>
webserver server; // create a instance of your server

CRC32 crc;
uint32_t checksum;

#if defined(ESP8266) && !defined(D5)
#define D5 (14)
#define D6 (12)
#define D7 (13)
#define D8 (15)
#endif

#ifdef ESP32
#define BAUD_RATE 57600
#else
#define BAUD_RATE 57600
#endif

SoftwareSerial serialPort;
#define SERIAL_BUFFER_MAX 128


// struct_message incomingReadings;
dataPacket packet;
unsigned char serialBuffer[SERIAL_BUFFER_MAX]; //buffer to hold incoming packet,

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {

  // Copies the sender mac address to a string
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  memcpy(&packet, incomingData, sizeof(packet));
  
  Serial.printf("Board ID            : %s \n", macStr);
  Serial.printf("Sensor Type         : %s \n", sensors[packet.sensorType]);
  Serial.printf("Sensor accuracy     : %d \n", packet.accuracy);
  Serial.printf("temperature         : %4.2f C\n", packet.temperature);
  Serial.printf("humidity            : %4.2f %%\n", packet.humidity);
  Serial.printf("pressure            : %.0f hPa\n", packet.pressure);
  Serial.printf("gasResistance       : %.0f Ohm\n", packet.gasResistance);
  Serial.printf("iaq                 : %.2f \n", packet.iaq);
  Serial.printf("staticIaq           : %.2f \n", packet.staticIaq);
  Serial.printf("co2Equivalent       : %.2f \n", packet.co2Equivalent);
  Serial.printf("breathVocEquivalent : %.2f \n", packet.breathVocEquivalent);
  Serial.println();

  server.update(macStr, &packet);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("-----------------------------------------------"));
  Serial.println(F("Healthy indoors project"));
  Serial.println(F("serial 2 http node"));
  Serial.println(F("version 0.0.1"));
  Serial.println(F("-----------------------------------------------"));

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print(F("Connecting to AP: "));
  Serial.print(ssid);
  Serial.print(F(" "));
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(F("."));
  }
  Serial.println(" connected");
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print(F("MAC address: "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("Wi-Fi Channel: "));
  Serial.println(WiFi.channel());
  Serial.println();

  // Init software serial
  Serial.println(F("initializing Serial Port on GPIO15 (D8) TX and GPIO13 (D7) RX"));
  Serial.print(F("Baud rate: "));
  Serial.println(BAUD_RATE);
  serialPort.begin(BAUD_RATE, D7, D8, SWSERIAL_8N1, false, 95, 11);
  
  // Init server
  server.init();
}
 
bool serialClean = false;

void loop() {
  // call server loop
  server.loop();

  // handle inputs on serial port
  if (serialPort.available() > 0) {
    if (!serialClean) {
      uint8_t serialInput = serialPort.read();
      Serial.print(serialInput);
      if (serialInput == 13 ) { // \r
        do {
          if (serialPort.available() > 0) {
            serialInput =  serialPort.read();
            Serial.print(serialInput);
          }
        } while (serialInput != 10); // \c
        Serial.println(F("end of package detected"));
        serialClean = true;
      }
    } else { // serial is "clean" so next package should be valid
      int count = 0;
      uint8_t packageLength = serialPort.read();
      packageLength+=HEADER_LENGTH; // add 6 for mac, 4 for checksum and 2 for \r\n
      Serial.println();
      // Serial.print(F("package start. Length: "));
      // Serial.println(packageLength);
      do {
        if (serialPort.available() > 0) {
          serialBuffer[count] = serialPort.read();
          count++;
          if (count >= SERIAL_BUFFER_MAX) {
            Serial.println(F("Buffer overflow! rescan"));
            serialClean = false;
            break;
          }
        }
      } while (count<packageLength);
      if (count == packageLength) {
        char macStr[18];
        Serial.print("Packet received from: ");
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           serialBuffer[0], serialBuffer[1], serialBuffer[2], serialBuffer[3], serialBuffer[4], serialBuffer[5]);
        Serial.print(macStr);
        Serial.print(F(" checksum: "));
        uint32_t checksumReceived =uint32_t((unsigned char)(serialBuffer[6]) |
            (unsigned char)(serialBuffer[7]) << 8 |
            (unsigned char)(serialBuffer[8]) << 16 |
            (unsigned char)(serialBuffer[9]) << 24);
        Serial.print(checksumReceived);
        checksum = CRC32::calculate(&serialBuffer[10], packageLength - HEADER_LENGTH);
        Serial.print(F(":"));
        Serial.print(checksum);
        if (checksumReceived == checksum) {
          Serial.println(F(" ok"));
          OnDataRecv(&serialBuffer[0], &serialBuffer[10], packageLength - HEADER_LENGTH);
        } else {
          Serial.println(F(" missmatch!"));
        }
      }
    }
  }
}