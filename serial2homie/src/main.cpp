/*
  Healthy Indoors Project

  serial 2 homie (MQTT)

  DESCRIPTION:
  the data from the esp-now local network received via the espnow2serial node is decoded and 
  published to an mqtt broker following the homie convention

  LIMITATIONS:
   - only unencrypted communication is tested
   
  IMPROVEMENTS
   - establish TLS encrypted communication
   - the serial in decoder should be looked over by an expert ;)

  This project was started September 2020 by Christian Meinert
  as a reaction of the global COVID-19 epidemic and the need of an as healthy as possible indoor enviornment
  as it became obvious that the virus is likely to be able to spread via aerosols which could stay longer
  in the air and travel further inside rooms as droplets.

  original project location on Github :

  Stay healthy, keep distance, open windows!
*/
#define VERSION "0.0.1"

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include "../../include/datatypes.h"
#include <SoftwareSerial.h>
#include <CRC32.h>

// edit all your credentials here:
#include "../../include/credentials.h"

// client to handle outbound traffic
#include <homieclient.h>
homieClient server; // create a instance of your server

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
  if (!server.rts()) return;
  digitalWrite(LED_BUILTIN, LOW);
  // Copies the sender mac address to a string
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  if (len>(int) sizeof(packet)) len = sizeof(packet);
  memcpy(&packet, incomingData, len);

#ifdef SERIAL_DEBUG  
  Serial.printf(" Packet length       : %d/%lu bytes\n", len, sizeof(packet));
  Serial.printf(" Board ID            : %s \n", macStr);
  Serial.printf(" Uptime              : %lu \n", packet.uptime);
  Serial.printf(" Sensor ID           : %d \n", packet.sensorType);
  Serial.printf(" Sensor Type         : %s \n", sensors[packet.sensorType]);
  Serial.printf(" Sensor accuracy     : %d \n", packet.accuracy);
  Serial.printf(" temperature         : %4.2f C\n", packet.temperature);
  Serial.printf(" humidity            : %4.2f %%\n", packet.humidity);
  Serial.printf(" pressure            : %.0f hPa\n", packet.pressure);
  Serial.printf(" gasResistance       : %.0f Ohm\n", packet.gasResistance);
  Serial.printf(" iaq                 : %.2f \n", packet.iaq);
  Serial.printf(" staticIaq           : %.2f \n", packet.staticIaq);
  Serial.printf(" co2Equivalent       : %.2f \n", packet.co2Equivalent);
  Serial.printf(" breathVocEquivalent : %.2f \n", packet.breathVocEquivalent);
  Serial.printf(" co2                 : %.2f \n", packet.co2);
  Serial.printf(" pm2.5               : %.2f \n", packet.pm25);
  Serial.printf(" Sensor accuracy     : %d \n", packet.accuracy);
  Serial.println();
#endif

  server.update(macStr, &packet);
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("-----------------------------------------------"));
  Serial.println(F("Healthy indoors project"));
  Serial.println(F("serial 2 homie mqtt broker"));
  Serial.println(F("version 0.0.1"));
  Serial.println(F("-----------------------------------------------"));

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
      Serial.println();
      uint8_t packageLength = serialPort.read();
      serialBuffer[0]=packageLength;
      uint8_t payloadLength = packageLength; // length of payload
      packageLength+=HEADER_LENGTH; // add 6 for mac, 4 for checksum and 2 for \r\n
      // Serial.print(F("package start. Length: "));
      // Serial.println(packageLength);
      int count = 1;
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
      } while (count<packageLength+1);
      Serial.println();
      if (count == packageLength+1) {
        char macStr[18];
        Serial.print("Packet received from: ");
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           serialBuffer[1], serialBuffer[2], serialBuffer[3], serialBuffer[4], serialBuffer[5], serialBuffer[6]);
        Serial.print(macStr);
        Serial.print(F(" bytes="));
        Serial.print(payloadLength);
        Serial.print(F(" checksum: "));
        uint32_t checksumReceived =uint32_t((unsigned char)(serialBuffer[7+payloadLength]) |
            (unsigned char)(serialBuffer[8+payloadLength]) << 8 |
            (unsigned char)(serialBuffer[9+payloadLength]) << 16 |
            (unsigned char)(serialBuffer[10+payloadLength]) << 24);
        Serial.print(checksumReceived);
        checksum = CRC32::calculate(serialBuffer, payloadLength+7); // payload and 1 byte length & 6 byte MAC address
        Serial.print("(");
        Serial.print(payloadLength+7);
        Serial.print(")");
        Serial.print(F(":"));
        Serial.print(checksum);
        if (checksumReceived == checksum) {
          Serial.println(F(" ok"));
          OnDataRecv(&serialBuffer[1], &serialBuffer[7], packageLength - HEADER_LENGTH );
        } else {
          Serial.println(F(" missmatch!"));
          Serial.println();
          serialClean=false; // rescan for CR/LF
        }
      }
    }
  }
}