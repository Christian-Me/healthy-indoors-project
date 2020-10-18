#ifndef S_BME860_H__
#define S_BME860_H__

#include <Arduino.h>
#ifdef USE_RTC
  #include "rtcmem.h"
#endif
#include "bsec.h"

class s_bme680{
  private:
    bool initialized = false;
    uint8_t sensorIndex = 0;
    uint16_t sensorInterval = 30; // Inteval for sensor readings in seconds
    unsigned long sensorTimeout = 0;
    String output;

    // sensor specific
    Bsec sensorBME680;
    uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
    uint16_t stateUpdateCounter = 0;

    #ifdef USE_RTC
      rtcmem rtcMem;
      rtcBufferType rtcBuffer;
    #endif
    void errLeds(void);
    void loadState(void);
    void updateState(void);
  public:
    s_bme680(uint8_t);
    bool checkSensorStatus(void);
    bool available(void);
    bool read(dataPacket*);
    bool init(uint16_t delaySec);
    void loop();
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

s_bme680::s_bme680(uint8_t index = 0) {
  sensorIndex = index;
  initialized = false;
}

bool s_bme680::init(uint16_t delaySec = 30) {
  sensorInterval=delaySec*1000;
  sensorTimeout=millis()+sensorInterval;

  uint8_t bme680Address = 0;
  Serial.println();
  Serial.println(F("Bosch Sensortec BME680"));
  Serial.println(F("-----------------------------------------------"));
  Serial.println(" Searching for I2C Devices ");
  Wire.begin();
  int count=0;
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print(" Found I2C Device: ");
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
    Serial.print(F(" BME680 found at 0x"));
    Serial.println(bme680Address, HEX);

    sensorBME680.begin(bme680Address, Wire);
    output = F(" BSEC library version ");
    output += String(sensorBME680.version.major) + F(".") + String(sensorBME680.version.minor) + F(".") + String(sensorBME680.version.major_bugfix) + F(".") + String(sensorBME680.version.minor_bugfix);
    Serial.println(output);
    checkSensorStatus();
    sensorBME680.setConfig(bsec_config_iaq);
    checkSensorStatus();


    loadState();


    bsec_virtual_sensor_t sensorList[11] = {
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
      BSEC_OUTPUT_STABILIZATION_STATUS
    };

    sensorBME680.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
    checkSensorStatus();

    initialized = true;
  } else {
    initialized = false;
  }
  return initialized;
}


bool s_bme680::read(dataPacket* packet)
{
  if (sensorTimeout<millis()) {
    sensorTimeout=millis()+sensorInterval;

    if (sensorBME680.run()) { // If new data is available
      unsigned long time_trigger = millis();
      #ifdef SERIAL_DEBUG
        output = F("----------------------------------------");
        output += F("\nTimestamp: ");
        output += String(time_trigger);
        output += F("\nAccuracy: ");
        output += String(sensorBME680.iaqAccuracy);
        output += F("\nTemperature: ");
        output += String(sensorBME680.temperature);
        output += F("\nHumidity: ");
        output += String(sensorBME680.humidity);
        output += F("\nPressure: ");
        output += String(sensorBME680.pressure);
        output += F("\nGas Resistance: ");
        output += String(sensorBME680.gasResistance);
        output += F("\nIAQ: ");
        output += String(sensorBME680.iaq);
        output += F("\nStatic IAQ: ");
        output += String(sensorBME680.staticIaq);
        output += F("\nCO2: ");
        output += String(sensorBME680.co2Equivalent);
        output += F("\nBreath VOC: ");
        output += String(sensorBME680.breathVocEquivalent);
        Serial.println(output);
      #endif

      updateState();

      initDataPacket(packet);
      packet -> sensorType= SENSOR_BME680;
      packet -> sensorIndex= sensorIndex;
      packet -> accuracy= sensorBME680.iaqAccuracy;
      packet -> temperature= sensorBME680.temperature;
      packet -> pressure= sensorBME680.pressure;
      packet -> humidity= sensorBME680.humidity;
      packet -> gasResistance= sensorBME680.gasResistance;
      packet -> iaq= sensorBME680.iaq;
      packet -> staticIaq= sensorBME680.staticIaq;
      packet -> co2Equivalent= sensorBME680.co2Equivalent;
      packet -> breathVocEquivalent= sensorBME680.breathVocEquivalent;

      return true;
    } else {
      checkSensorStatus();
      return false;
    }
  }
  return false;
}

bool available(void) {
  return true;
};

bool s_bme680::checkSensorStatus(void) {
  if (sensorBME680.status != BSEC_OK) {
    if (sensorBME680.status < BSEC_OK) {
      output = "BSEC error code : " + String(sensorBME680.status);
      Serial.println(output);
    } else {
      output = "BSEC warning code : " + String(sensorBME680.status);
      Serial.println(output);
    }
    return false;
  }

  if (sensorBME680.bme680Status != BME680_OK) {
    if (sensorBME680.bme680Status < BME680_OK) {
      output = "BME680 error code : " + String(sensorBME680.bme680Status);
      Serial.println(output);
    } else {
      output = "BME680 warning code : " + String(sensorBME680.bme680Status);
      Serial.println(output);
    }
  }
  return true;
}

void s_bme680::errLeds(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

// loading calibration data from non volatile memory
void s_bme680::loadState(void) {
  #ifdef USE_RTC
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
    sensorBME680.setState(bsecState);
    checkSensorStatus();
  } else {
    Serial.print(F("no data found: Erasing rtc memory "));
    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE + 1; i+=RTC_BLOCK_SIZE) {
        rtcMem.write32(floor(i / RTC_BLOCK_SIZE),0);
        Serial.print(F("."));
    }
    Serial.println();
  }
  #endif
}

// save calibration data to non volatile memory
void s_bme680::updateState(void) {
  #ifdef USE_RTC
  bool update = false;
  /* Set a trigger to save the state. Here, the state is saved every STATE_SAVE_PERIOD with the first state being saved once the algorithm achieves full calibration, i.e. iaqAccuracy = 3 */
  if (stateUpdateCounter == 0) {
    if (sensorBME680.iaqAccuracy >= 3) {
      Serial.print(F("State counter: "));
      Serial.print(stateUpdateCounter);
      Serial.print(F("Accuracy: "));
      Serial.println(sensorBME680.iaqAccuracy);
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
    checkSensorStatus();

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
  #endif
}

void s_bme680::loop() {
  
}

#endif