#include "rtcmem.h"
#include "bsec.h"

class BME680 {
    bool initialized = false;
    Bsec iaqSensor;
    uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
    uint16_t stateUpdateCounter = 0;
    rtcmem rtcMem;
    rtcBufferType rtcBuffer;
    void checkIaqSensorStatus(void);
    void errLeds(void);
    void loadState(void);
    void updateState(void);
  public:
    BME680(void);
    bool sensorInit(void);
    bool sensorRead(dataPacket*);
};

/* Configure the BSEC library with information about the sensor
    18v/33v = Voltage at Vdd. 1.8V or 3.3V
    3s/300s = BSEC operating mode, BSEC_SAMPLE_RATE_LP or BSEC_SAMPLE_RATE_ULP
    4d/28d = Operating age of the sensor in days
    generic_18v_3s_4d
    generic_18v_3s_28d
    generic_18v_300s_4d
    generic_18v_300s_28d
    generic_33v_3s_4d
    generic_33v_3s_28d
    generic_33v_300s_4d
    generic_33v_300s_28d
*/
const uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_3s_4d/bsec_iaq.txt"
};

#define STATE_SAVE_PERIOD	UINT32_C(360 * 60 * 1000) // 360 minutes - 4 times a day
String output;

// Entry point for the example
BME680::BME680(void) {
  initialized = false;
}

bool BME680::sensorInit(void) {
  uint8_t bme680Address = 0;

  Wire.begin();
  Serial.println();
  Serial.println(F("Bosch Sensortec BME680"));
  Serial.println(F("-----------------------------------------------"));
  Serial.println("Searching for I2C Devices ");
  int count=0;
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0)
      {
      Serial.print("Found I2C Device: ");
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.print(")");
      if (i==BME680_I2C_ADDR_SECONDARY || i==BME680_I2C_ADDR_PRIMARY) {
        Serial.print(" BME680");
        bme680Address=i;
      }
      Serial.println();
      count++;
      delay(1);
      }
  }

  if (bme680Address==BME680_I2C_ADDR_SECONDARY || bme680Address==BME680_I2C_ADDR_PRIMARY) {
    Serial.print(F("BME680 found at 0x"));
    Serial.println(bme680Address, HEX);

    iaqSensor.begin(bme680Address, Wire);
    output = F("BSEC library version ");
    output += String(iaqSensor.version.major) + F(".") + String(iaqSensor.version.minor) + F(".") + String(iaqSensor.version.major_bugfix) + F(".") + String(iaqSensor.version.minor_bugfix);
    Serial.println(output);
    checkIaqSensorStatus();
    iaqSensor.setConfig(bsec_config_iaq);
    checkIaqSensorStatus();

    #ifdef USE_RTC
      loadState();
    #endif

    bsec_virtual_sensor_t sensorList[10] = {
      BSEC_OUTPUT_RAW_TEMPERATURE,
      BSEC_OUTPUT_RAW_PRESSURE,
      BSEC_OUTPUT_RAW_HUMIDITY,
      BSEC_OUTPUT_RAW_GAS,
      BSEC_OUTPUT_IAQ,
      BSEC_OUTPUT_STATIC_IAQ,
      BSEC_OUTPUT_CO2_EQUIVALENT,
      BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    };

    iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
    checkIaqSensorStatus();

    initialized = true;
  } else {
    initialized = false;
  }
  return initialized;
}

// Function that is looped forever
bool BME680::sensorRead(dataPacket* packet)
{
  unsigned long time_trigger = millis();
  if (iaqSensor.run()) { // If new data is available
    #ifdef SERIAL_DEBUG
      output = F("----------------------------------------");
      output += F("\nTimestamp: ");
      output += String(time_trigger);
      output += F("\nAccuracy: ");
      output += String(iaqSensor.iaqAccuracy);
      output += F("\nTemperature: ");
      output += String(iaqSensor.temperature);
      output += F("\nHumidity: ");
      output += String(iaqSensor.humidity);
      output += F("\nPressure: ");
      output += String(iaqSensor.pressure);
      output += F("\nGas Resistance: ");
      output += String(iaqSensor.gasResistance);
      output += F("\nIAQ: ");
      output += String(iaqSensor.iaq);
      output += F("\nStatic IAQ: ");
      output += String(iaqSensor.staticIaq);
      output += F("\nCO2: ");
      output += String(iaqSensor.co2Equivalent);
      output += F("\nBreath VOC: ");
      output += String(iaqSensor.breathVocEquivalent);
      Serial.println(output);
    #endif
    #ifdef USE_RTC
      updateState();
    #endif
    packet -> sensorType= SENSOR_BME680;
    packet -> sensorIndex=0; // default
    packet -> accuracy= iaqSensor.iaqAccuracy;
    packet -> temperature= iaqSensor.temperature;
    packet -> pressure= iaqSensor.pressure;
    packet -> humidity= iaqSensor.humidity;
    packet -> gasResistance= iaqSensor.gasResistance;
    packet -> iaq= iaqSensor.iaq;
    packet -> staticIaq= iaqSensor.staticIaq;
    packet -> co2Equivalent= iaqSensor.co2Equivalent;
    packet -> breathVocEquivalent= iaqSensor.breathVocEquivalent;
    return true;
  } else {
    checkIaqSensorStatus();
    return false;
  }
}

// Helper function definitions
void BME680::checkIaqSensorStatus(void)
{
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      output = "BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    }
  }
  iaqSensor.status = BSEC_OK;
}

void BME680::errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

void BME680::loadState(void)
{
  uint32_t blobSize = rtcMem.read32(0);
  Serial.print(F("Reading state from RTC ("));
  Serial.print(blobSize);
  Serial.print(F(":"));
  Serial.print(BSEC_MAX_STATE_BLOB_SIZE);
  Serial.println(F(")"));

  if (blobSize == BSEC_MAX_STATE_BLOB_SIZE) {
    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i+=RTC_BLOCK_SIZE) {
      rtcBuffer.dw = (uint32_t) rtcMem.read32(floor(i / RTC_BLOCK_SIZE) + 1);
      Serial.print(floor(i / RTC_BLOCK_SIZE) + 1, 0);
      Serial.print(F(": "));
      Serial.print(rtcBuffer.bytes.b[0], HEX);
      Serial.print(F(" "));
      Serial.print(rtcBuffer.bytes.b[1], HEX);
      Serial.print(F(" "));
      Serial.print(rtcBuffer.bytes.b[2], HEX);
      Serial.print(F(" "));
      Serial.println(rtcBuffer.bytes.b[3], HEX);
      for (uint8_t b = 0; b<4; b++) {
        if (i+b < BSEC_MAX_STATE_BLOB_SIZE) {
          bsecState[i+b]=rtcBuffer.bytes.b[b];
        }
      }
    }
    iaqSensor.setState(bsecState);
    checkIaqSensorStatus();
  } else {
    Serial.print(F("no data found: Erasing rtc memory "));
    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE + 1; i+=RTC_BLOCK_SIZE) {
        rtcMem.write32(floor(i / RTC_BLOCK_SIZE),0);
        Serial.print(F("."));
    }
    Serial.println();
  }
}

void BME680::updateState(void)
{
  bool update = false;
  /* Set a trigger to save the state. Here, the state is saved every STATE_SAVE_PERIOD with the first state being saved once the algorithm achieves full calibration, i.e. iaqAccuracy = 3 */
  if (stateUpdateCounter == 0) {
    if (iaqSensor.iaqAccuracy >= 3) {
      Serial.print(F("State counter: "));
      Serial.print(stateUpdateCounter);
      Serial.print(F("Accuracy: "));
      Serial.println(iaqSensor.iaqAccuracy);
      update = true;
      stateUpdateCounter++;
    }
  } else {
    /* Update every STATE_SAVE_PERIOD milliseconds */
    if ((stateUpdateCounter * STATE_SAVE_PERIOD) < millis()) {
      update = true;
      stateUpdateCounter++;
    }
  }

  if (update) {
    iaqSensor.getState(bsecState);
    checkIaqSensorStatus();

    Serial.println(F("Writing state to RTC Memory"));

    uint8_t rtcBlock=1;
    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE ; i+=RTC_BLOCK_SIZE) {
        Serial.print((rtcBlock > 9) ? 0 : "");
        Serial.print(rtcBlock);
        Serial.print(F(": "));
        for (uint8_t b = 0; b<4; b++) {
          if (i+b < BSEC_MAX_STATE_BLOB_SIZE) {
            if (bsecState[i+b]<10) {
              Serial.print(F("0"));
            }
            Serial.print(bsecState[i+b], HEX);
            Serial.print(F(" "));
            rtcBuffer.bytes.b[b]=bsecState[i+b];
          }
        }
        rtcMem.write32(rtcBlock, rtcBuffer.dw);
        rtcBlock++;
        Serial.println(F(" ok."));
    }
    rtcMem.write32(0, BSEC_MAX_STATE_BLOB_SIZE);
  }
}