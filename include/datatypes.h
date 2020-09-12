#include <Arduino.h>
#ifdef ESP32
  #include <esp_now.h>
#else
  #include <espnow.h>
#endif

#define HEADER_LENGTH 12    // header length for serial communication: 6 for mac, 4 for checksum and 2 for \r\n

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
};

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