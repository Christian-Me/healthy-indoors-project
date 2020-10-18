#ifndef S_CCS811_H__
#define S_CCS811_H__

#include <Arduino.h>
#include "rtcmem.h"
#include <Wire.h>

// #include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include "Adafruit_CCS811.h"
#define CCS811_ADDR_PRIMARY 0x5B    //Default I2C Address
#define CCS811_ADDR_SECONDARY 0x5A  //Alternate I2C Address

Adafruit_CCS811 mySensor;

class s_ccs811{
    bool initialized = false;
    uint8_t sensorIndex = 0;
    uint16_t sensorInterval = 30000; // Inteval for sensor readings in seconds
    unsigned long sensorTimeout = 0;
    String output;
    rtcmem rtcMem;
    rtcBufferType rtcBuffer;
    void checkSensorStatus(void);
    void errLeds(void);
    void loadState(void);
    void updateState(void);
  public:
    s_ccs811(uint8_t);
    bool available(void);
    bool read(dataPacket*);
    bool init(uint16_t delaySec);
    void loop();
};


#define STATE_SAVE_PERIOD	UINT32_C(360 * 60 * 1000) // 360 minutes - 4 times a day

s_ccs811::s_ccs811(uint8_t index = 0) {
  sensorIndex = index;
  initialized = false;
}

bool s_ccs811::init(uint16_t delaySec = 30) {
  uint8_t sensorAddress = 0;
  sensorInterval=delaySec*1000;
  sensorTimeout=millis()+sensorInterval;
  Wire.begin();
  Serial.println();
  Serial.println(F("Sciosense CCS811"));
  Serial.println(F("-----------------------------------------------"));
  Serial.println(" Searching for I2C Devices ");
  int count=0;
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0)
      {
      Serial.print(" Found I2C Device: ");
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.print(")");
      if (i==CCS811_ADDR_PRIMARY || i==CCS811_ADDR_SECONDARY) {
        Serial.print(" ccs811");
        sensorAddress=i;
      }
      Serial.println();
      count++;
      delay(1);
      }
  }

  if (sensorAddress==CCS811_ADDR_SECONDARY || sensorAddress==CCS811_ADDR_PRIMARY) {
    Serial.print(F(" s_ccs811 found at 0x"));
    Serial.println(sensorAddress, HEX);

    if (mySensor.begin() == false)
    {
      Serial.print(" CCS811 error. Please check wiring.");
      return false;
    }

    checkSensorStatus();

    #ifdef USE_RTC
      loadState();
    #endif
    initialized = true;
  } else {
    initialized = false;
  }
  return initialized;
}


bool s_ccs811::read(dataPacket* packet)
{
  if (sensorTimeout<millis()) {
    sensorTimeout=millis()+sensorInterval;
    if (mySensor.available()) { // If new data is available
      unsigned long time_trigger = millis();
      mySensor.readData();

      #ifdef SERIAL_DEBUG
        output = F("CCS811 Sensor");
        output += F("\n Timestamp: ");
        output += String(time_trigger);
        output += F("\n eCO2: ");
        output += String(mySensor.geteCO2());
        output += F("\n TVOC: ");
        output += String(mySensor.getTVOC());
        Serial.println(output);
      #endif
      #ifdef USE_RTC
        updateState();
      #endif

      initDataPacket(packet);
      packet -> sensorType= SENSOR_CCS811;
      packet -> sensorIndex= sensorIndex; // default

      packet -> co2Equivalent= mySensor.geteCO2();
      packet -> breathVocEquivalent= mySensor.getTVOC();
      packet -> iaq= voc2IAQ(packet -> breathVocEquivalent);
      packet -> staticIaq= voc2IAQ(packet -> breathVocEquivalent);

      return true;
    } else {
      Serial.println(F("CCS811 no data available!"));
      checkSensorStatus();
      return false;
    }
  } 
  return false;
}


void s_ccs811::checkSensorStatus(void) {
  if (mySensor.checkError()) {
    Serial.println("CCS811 error.");
  }
}

void s_ccs811::errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

// loading calibration data from non volatile memory
// not implemented jet 
void s_ccs811::loadState(void)
{
}

// save calibration data to non volatile memory
// not implemented jet 
void s_ccs811::updateState(void)
{
}


void s_ccs811::loop() {
  
}

#endif