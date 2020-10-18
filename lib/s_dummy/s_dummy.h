#ifndef S_DUMMY_H__
#define S_DUMMY_H__

#include <Arduino.h>

class s_dummy{
  private:
    bool initialized = false;
    uint8_t sensorIndex = 0;
    uint16_t sensorInterval = 30; // Inteval for sensor readings in seconds
    unsigned long sensorTimeout = 0;
    String output;
    void checkSensorStatus(void);
    void errLeds(void);
    void loadState(void);
    void updateState(void);
  public:
    s_dummy(uint8_t);
    bool available(void);
    bool read(dataPacket*);
    bool init(uint16_t delaySec);
    void loop();
};


s_dummy::s_dummy(uint8_t index = 0) {
  sensorIndex = index;
  initialized = false;
}

bool s_dummy::init(uint16_t delaySec = 30) {

  Serial.println();
  Serial.println(F("Dummy Sensor"));
  Serial.println(F("-----------------------------------------------"));
  Serial.println(F(" some usefull information on console"));

  // init your sensor here 

  sensorInterval=delaySec*1000;
  sensorTimeout=millis()+sensorInterval;
  initialized = true;
  return initialized;
}


bool s_dummy::read(dataPacket* packet)
{
  if (sensorTimeout<millis()) {
    sensorTimeout=millis()+sensorInterval;

    // read your sensor here

    // dump data to console
    unsigned long time_trigger = millis();
    #ifdef SERIAL_DEBUG
      output = F("Dummy Sensor");
      output += F("\n Timestamp: ");
      output += String(time_trigger);
      output += F("\n CO2: ");
      output += String(0);
      Serial.println(output);
    #endif

    // initialze packet struct
    initDataPacket(packet);
    packet -> sensorType= SENSOR_UNKNOWN; // as defined in datatypes.h
    packet -> sensorIndex= sensorIndex;

    // reüöace UNSUPPORTED_VALUE by sensor read function. Delete unneeded properties
    packet -> accuracy= UNSUPPORTED_VALUE;
    packet -> temperature= UNSUPPORTED_VALUE;
    packet -> pressure= UNSUPPORTED_VALUE;
    packet -> humidity= UNSUPPORTED_VALUE;
    packet -> gasResistance= UNSUPPORTED_VALUE;
    packet -> iaq= UNSUPPORTED_VALUE;
    packet -> staticIaq= UNSUPPORTED_VALUE;
    packet -> co2Equivalent= 0;
    packet -> breathVocEquivalent= 0;
    packet -> co2= UNSUPPORTED_VALUE;
    packet -> pm25= UNSUPPORTED_VALUE;

    // do not change
    packet -> uptime= millis()/1000;
    packet -> battery= 90;     // ToDo: replace by real battery levels

    //check if a error accrued before
    return false;
  }
  return false;
}

// Helper function definitions
void s_dummy::checkSensorStatus(void) {
}

void s_dummy::errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

// loading calibration data from non volatile memory
void s_dummy::loadState(void)
{
}

// save calibration data to non volatile memory
void s_dummy::updateState(void)
{
}


void s_dummy::loop() {
  
}
#endif