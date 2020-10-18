#ifndef DATATYPES_H__
#define DATATYPES_H__

#define HEADER_LENGTH 12    // header length for serial communication: 6 for mac, 4 for checksum and 2 for \r\n
#define UNSUPPORTED_VALUE 0  // value indicating a unsupported feature by the sensor

struct __attribute__((packed)) dataPacket {
    uint8_t channel;                // wifi channel
    uint8_t state;                  // 0 = OK
    uint8_t battery;                // battery level in %
    uint8_t sensorType;             // see below
    uint8_t sensorIndex;            // if more than one sensor of the same type is attached
    uint8_t accuracy;               // accuracy value according to sensor datasheet
    float pressure;
    float temperature;
    float humidity;
    float gasResistance;
    float iaq;
    float staticIaq;
    float co2Equivalent;
    float breathVocEquivalent;
    float co2;
    float pm25;
    unsigned long uptime;
};

/*!
   @brief    Fill data packet with default values
    @param    packet  pointer to dataPackage struct.
*/
void initDataPacket(dataPacket* packet) {
  // sensor specific values
    packet -> sensorType= UNSUPPORTED_VALUE;
    packet -> sensorIndex= UNSUPPORTED_VALUE;
    packet -> accuracy= UNSUPPORTED_VALUE;
    packet -> temperature= UNSUPPORTED_VALUE;
    packet -> pressure= UNSUPPORTED_VALUE;
    packet -> humidity= UNSUPPORTED_VALUE;
    packet -> gasResistance= UNSUPPORTED_VALUE;
    packet -> iaq= UNSUPPORTED_VALUE;
    packet -> staticIaq= UNSUPPORTED_VALUE;
    packet -> co2Equivalent= UNSUPPORTED_VALUE;
    packet -> breathVocEquivalent= UNSUPPORTED_VALUE;
    packet -> co2= UNSUPPORTED_VALUE;
    packet -> pm25= UNSUPPORTED_VALUE;
  // device specific values
    packet -> uptime= millis()/1000;
    packet -> battery= 0;
}

// callback type for button press
typedef void (*buttonCallback) (uint8_t,uint8_t);

// callback type for draw a single chart sample
typedef void (*drawSampleCallback) (uint8_t, int, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, float, uint16_t);

// callback type for determin draw color
typedef uint32_t (*colorCallback) (float);

// callback type for determin alarm states
typedef uint8_t (*alarmCallback) (float);

// callback type for determin draw color
typedef void (*viewportCallback) (uint16_t x, uint16_t y, uint16_t w, uint16_t h);

#define SENSOR_UNKNOWN 0
#define SENSOR_BME680 1
#define SENSOR_SCD30 2
#define SENSOR_MHZ19 3
#define SENSOR_CCS811 4

typedef char sensorString [10];
sensorString sensors [5] = {
  "n/a",
  "BME680",
  "SCD30",
  "MHZ19",
  "CCS811"
};

const uint8_t maxBreakpoints = 6;
const double AQIbreakpoints[6] = {50,100,150,200,300,400}; // good, moderate, unhealthy for sensitive, unhealthy, very unhealthy, hazardous, extremely hazardous
const double CO2breakpoints[6] = {500,1000,1500,2000,3000,5000};

// instead of a linear conversion a smooth curve (spline) would be nice
float value2iaq(double value, const double breakpoints[6]) {
  for (uint8_t i=0; i<maxBreakpoints; i++) {

  }
  return value;
}

float co2IAQ(float value) {
  return value2iaq(value, CO2breakpoints);
};

float voc2IAQ(float co2) {
  return 25;
};


void printDataPackage(const uint8_t *data, int len) {
  dataPacket packet;
  if (len>(int) sizeof(packet)) len = sizeof(packet);
  memcpy(&packet, data, len);

  Serial.printf(" Packet length       : %d/%i bytes\n", len, sizeof(packet));
  Serial.printf(" Channel             : %d \n", packet.channel);
  Serial.printf(" State               : %d \n", packet.state);
  Serial.printf(" Battery             : %d \n", packet.battery);
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
}

#endif