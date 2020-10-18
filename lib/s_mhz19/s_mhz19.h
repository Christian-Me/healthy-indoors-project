#ifndef S_MHZ19_H__
#define S_MHZ19_H__

#include <Arduino.h>
#include <MHZ19.h>
#include <SoftwareSerial.h>

#define RX_PIN 15           // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 13           // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600       // Device to MH-Z19 Serial baudrate (should not be changed)

MHZ19 sensorMHZ19;
SoftwareSerial softwareSerial;

class s_mhz19{
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
    s_mhz19(uint8_t);
    bool available(void);
    bool read(dataPacket*);
    bool init(uint16_t delaySec);
    void loop();
};


s_mhz19::s_mhz19(uint8_t index = 0) {
  sensorIndex = index;
  initialized = false;
}

bool s_mhz19::init(uint16_t delaySec = 30) {

  Serial.println();
  Serial.println(F("Winsen-Sensors MHZ19"));
  Serial.println(F("-----------------------------------------------"));
  Serial.print(F(" Serial Baud Rate : "));
  Serial.println(BAUDRATE);
  Serial.print(F(" RX pin           : "));
  Serial.println(RX_PIN);
  Serial.print(F(" TX pin           : "));
  Serial.println(TX_PIN);

  softwareSerial.begin(BAUDRATE, RX_PIN, TX_PIN, SWSERIAL_8N1, false, 95, 11);
  sensorMHZ19.begin(softwareSerial);
  sensorMHZ19.autoCalibration();

  sensorInterval=delaySec*1000;
  sensorTimeout=millis()+sensorInterval;
  initialized = true;
  return initialized;
}


bool s_mhz19::read(dataPacket* packet)
{
  if (sensorTimeout<millis()) {
    sensorTimeout=millis()+sensorInterval;

    unsigned long time_trigger = millis();

    #ifdef SERIAL_DEBUG
      output = F("MHZ19 Sensor");
      output += F("\n Timestamp: ");
      output += String(time_trigger);
      output += F("\n CO2: ");
      output += String(sensorMHZ19.getCO2());
      output += F("\n temperature: ");
      output += String(sensorMHZ19.getTemperature());
      output += F("\n accuracy: ");
      output += String(sensorMHZ19.getAccuracy());
      Serial.println(output);
    #endif
    #ifdef USE_RTC
      updateState();
    #endif

    initDataPacket(packet);
    packet -> sensorType= SENSOR_MHZ19;
    packet -> sensorIndex= sensorIndex;
    packet -> accuracy= sensorMHZ19.getAccuracy();
    packet -> temperature= sensorMHZ19.getTemperature();
    packet -> co2= sensorMHZ19.getCO2();
    //packet -> iaq= co2IAQ(packet->co2);
    packet -> co2Equivalent= sensorMHZ19.getCO2();

    //check if a error accrued before
    if (sensorMHZ19.errorCode==RESULT_OK) {
      return true;
    } else {
      checkSensorStatus();
      return false;
    } 
  }
  return false;
}


void s_mhz19::checkSensorStatus(void) {
  uint8_t result = sensorMHZ19.errorCode;
  if (result != RESULT_OK) {
    Serial.print(F("MHZ19 error "));
    Serial.println(result);
  }
}

void s_mhz19::errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

// loading calibration data from non volatile memory
void s_mhz19::loadState(void)
{
}

// save calibration data to non volatile memory
void s_mhz19::updateState(void)
{
}


void s_mhz19::loop() {
  
}
#endif