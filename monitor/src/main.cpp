#define MODULE_NAME "espnow2monitor"
#include <Arduino.h>
#include "../../include/datatypes.h"
#include "utils.h"
#ifdef ESP32 // nicer LOG output for ESP32
#include "esp32-hal-log.h"
#endif

// struct_message incomingReadings;
dataPacket packet;

// data collection and storage
#include "datacollection.h"
dataCollection dataStorage; // don't change this name because it is directly referenced by other classes (ToDo: fix this;)

// communication plugin
#ifdef COMUNICATION_ESPNOW
  #include "c_espnow.h"
  c_espnow comm1;
#endif

// sensor node specific libraries

#define USE_BME680
//#define USE_CCS811
#define USE_MHZ19

#ifdef USE_BME680
  #include "s_bme680.h"
  s_bme680 bme680;
#endif
#ifdef USE_CCS811
  #include "s_ccs811.h"
  s_ccs811 ccs811;
#endif
#ifdef USE_MHZ19
  #include "s_mhz19.h"
  s_mhz19 mhz19;
#endif

// monitor node specific libraries
#ifdef MONITOR_NEOPIXEL
  #include "m_neopixel.h"
  m_neopixel neopixel;
#endif
#ifdef MONITOR_EPAPER
  #define MATRIX_DISPLAY
  #include "epaperDisplay.h"
  epaperDisplay matrixDisplay;
  #include "chart.h"
  #include "ui.h"
#endif
#ifdef MONITOR_TFT
  #define MATRIX_DISPLAY
  #include "tftDisplay.h"
  tftDisplay matrixDisplay;
  #include "chart.h"
  chart iaqChart;
  chart humidityChart;
  chart temperatureChart;
  chart co2Chart;
  uint8_t currentChart = 0;
  #include "ui.h"
  ui mainUi;
#endif


void updateUI(void) {
  iaqChart.addSample("iaq");
  humidityChart.addSample("humidity");
  temperatureChart.addSample("temperature");
  co2Chart.addSample("co2");
}

// handle incoming data

void dataReceived() {
  digitalWrite(LED_BUILTIN, LOW);
  dataStorage.update((char *)WiFi.macAddress().c_str(), &packet);

  updateUI();

  digitalWrite(LED_BUILTIN, HIGH);
}

void dataReceived(const uint8_t *mac_addr, const uint8_t *data, int dataLength) {

  digitalWrite(LED_BUILTIN, LOW);

  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  size_t length = dataLength;
  if (sizeof(packet)<length) length=sizeof(packet); // limit length to maximum length of package if received package is bigger than the data structure of this node.
  initDataPacket(&packet); // fill packet with defaults (necessary is incoming packit is smaller than received package)
  memcpy(&packet, data, length);

  dataStorage.update(macStr, &packet);

  updateUI();

  digitalWrite(LED_BUILTIN, HIGH);
}

void button1Handler(uint8_t button, uint8_t event) {
  comm1.pauseReceive();
  if (matrixDisplay.getBacklight()==0) {
    matrixDisplay.setBacklight(BACKLIGHT_BRIGHTNESS);
    matrixDisplay.resetScreensaver();
  } else {
    currentChart++;
    if (currentChart>3) currentChart=0;
    switch (currentChart) {
      case 0: // IAQ
        Serial.println(F("Show IAQ chart"));
        iaqChart.visible(true);
        humidityChart.visible(false);
        temperatureChart.visible(false);
        co2Chart.visible(false);
        break;
      case 1: // Humidity
        Serial.println(F("Show Humidity chart"));
        iaqChart.visible(false);
        humidityChart.visible(true);
        temperatureChart.visible(false);
        co2Chart.visible(false);
        break;
      case 2: // Temperature
        Serial.println(F("Show Temperature chart"));
        iaqChart.visible(false);
        humidityChart.visible(false);
        temperatureChart.visible(true);
        co2Chart.visible(false);
        break;
      case 3: // CO2
        Serial.println(F("Show CO2 chart"));
        iaqChart.visible(false);
        humidityChart.visible(false);
        temperatureChart.visible(false);
        co2Chart.visible(true);
        break;
    }
    // comm1.resumeReceive();
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);     // initialize serial port

  Serial.println();
  Serial.println(F("-----------------------------------------------"));
  Serial.println(F("Healthy indoor project"));
  Serial.println(F("Node: Monitor"));
  #ifdef MONITOR_NEOPIXEL
    Serial.println(F("Display: neopixel"));
  #endif
  #ifdef MONITOR_EPAPER
    Serial.println(F("Display: epaper"));
  #endif
  #ifdef MONITOR_TFT
    Serial.println(F("Display: TFT"));
  #endif
  Serial.println(F("Version 0.0.1"));
  Serial.println(F("-----------------------------------------------"));
  Serial.println();
  dataStorage.init();

  // init sensor nodes
  #ifdef USE_BME680
    bme680.init();
  #endif
  #ifdef USE_CCS811
    ccs811.init();
  #endif
  #ifdef USE_MHZ19
    mhz19.init();
  #endif

  #ifdef MATRIX_DISPLAY // init matrix display first for splash screen
    matrixDisplay.init();
    matrixDisplay.registerButtonCallback(&button1Handler);
    mainUi.splash();
    unsigned long timer = millis() + 2000;
    while (timer>millis()) {
      matrixDisplay.loop(); // let the display do it`s thing as log as we wait
    }
    display.fillScreen(TFT_BLACK);
  #endif

  #ifdef MONITOR_NEOPIXEL // using neopixel and setting the brightness value
    neopixel.init(27, "iaq", 1, NEO_RGBW, 50);
  #endif

  #ifdef MATRIX_DISPLAY // init chart & ui for matrix dispaly
    iaqChart.init(F("Indoor Air Quality"),true, 20, 104,0,display.width()-104,display.height(),0,500,&getIAQColor,nullptr,&getIAQAlarm, &drawColorBar);
    humidityChart.init(F("Humidity"), false, 20, 104,0,display.width()-104,display.height(),0,100,&getHumidityColor,nullptr,&getHumidityAlarm, &drawColorLine);
    temperatureChart.init(F("Temperature"),false, 20, 104,0,display.width()-104,display.height(),0,45,&getTemperatureColor,nullptr,&getTemperatureAlarm, &drawColorLine);
    co2Chart.init(F("CO2"),false, 20, 104,0,display.width()-104,display.height(),0,4000,&getCO2Color,nullptr,&getCO2Alarm, &drawColorLine);
    mainUi.init();
  #endif

  #ifdef COMUNICATION_ESPNOW // using esp-now
    // 1. register callback for receiveing data
    comm1.registerReceiveCallback(&dataReceived);
    // 2. initialize espnow
    comm1.init("espnow2epaper");
  #endif
  Serial.println(F("Done."));
  Serial.println();

}

void loop() {
  #ifdef COMUNICATION_ESPNOW  // communicator loop
    comm1.loop();
  #endif
  #ifdef MONITOR_NEOPIXEL // neopixel loop
    neopixel.loop();
  #endif
  #ifdef MONITOR_EPAPER // epaper loop
    matrixDisplay.loop();
    mainUi.loop();    // refresh ui
    iaqChart.loop(); // refresh chart
    humidityChart.loop(); // refresh chart
    temperatureChart.loop(); // refresh chart
  #endif
  #ifdef MATRIX_DISPLAY // matrix display loop
    matrixDisplay.loop();
    mainUi.loop();    // refresh ui
    iaqChart.loop(); // refresh chart
    humidityChart.loop(); // refresh chart
    temperatureChart.loop(); // refresh chart
  #endif
  #ifdef USE_BME680
    if (bme680.read(&packet)) {
      comm1.send(&packet);
      dataReceived(); // write to data collection and update charts
    }
  #endif
  #ifdef USE_CCS811
    if (ccs811.read(&packet)) {
      comm1.send(&packet);
      dataReceived();
    }
  #endif
  #ifdef USE_MHZ19
    if (mhz19.read(&packet)) {
      comm1.send(&packet);
      dataReceived();
    }
  #endif
}
