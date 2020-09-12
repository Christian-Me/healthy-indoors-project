#define MODULE_NAME "espnow2serial"

// communication plugin
#include "c_espnow.h"

// module specific libraries
#include <SoftwareSerial.h>
#include <CRC32.h>

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
CRC32 crc;
uint32_t checksum;



// transmit data received by the communicator to serial
void dataReceived(uint8_t *mac_addr, uint8_t *data, uint8_t dataLength) {
    unsigned char serialPackage[dataLength+7]; // buffer for complete package (without checksum & CR/LF)
    serialPackage[0]=dataLength;      
    memcpy(&serialPackage[1], mac_addr,6);
    memcpy(&serialPackage[7], data, dataLength);
    checksum = CRC32::calculate(serialPackage,dataLength+7);
    
    serialPort.write(dataLength);       // length of the data package
    serialPort.write(mac_addr,6);       // mac address of the sender for id
    serialPort.write(data,dataLength);  // data package
    for (uint8_t i=0;i<=24;i+=8) {      // checksum of the data package as 4 bytes at the end
      uint8_t byteToWrite = (checksum >> i) & 0xFF;
      serialPort.write(byteToWrite);
    }
    serialPort.write("\r\n");           // like in the good old days
    Serial.printf("written to serial %d/%d\n",dataLength,checksum);
    digitalWrite(LED_BUILTIN, HIGH);
}
 
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);     // initialize serial port

  Serial.println();
  Serial.println(F("-----------------------------------------------"));
  Serial.println(F("Healthy indoor project"));
  Serial.println(F("espnow2serial bridge"));
  Serial.println(F("Version 0.0.1"));
  Serial.println(F("-----------------------------------------------"));
  Serial.println();
  Serial.println(F("Initializing Serial Port"));
  Serial.println(F(" GPIO15 (D8) TX and GPIO13 (D7) RX"));
  Serial.println(F(" Baud rate: "));
  Serial.println(BAUD_RATE);
  serialPort.begin(BAUD_RATE, D7, D8, SWSERIAL_8N1, false, 95, 11);
  Serial.println();

  // using esp-now:
  // initialize communicator
  c_espnow_init("espnow2serial");
  // register callback for receiveing data
  c_espnow_registerReceiveCB(&dataReceived);

  Serial.println(F("Done."));
  Serial.println();
}

void loop() {

  // communicator loop
  c_espnow_loop();
}
