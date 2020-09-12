
#define SAMPLE_PROPERTY "IAQ"   // id of property to be sampled
#define MAX_SAMPLED_DEVICES 5   // maximum device to sample
#define MAX_SAMPLES 190         // sample memory for selected values display width / 8
#define SAMPLE_MIN 0            // minimum of sampled values
#define SAMPLE_MAX 500          // maximum of sampled values
#define PIXEL_PER_SAMPLE 2      // 
#define CHART_REFRESH 10000     // refresh every 10sec (min 2x sample rate of your sensors)
unsigned long chartRefresh = 0;

// chart window for 296 x 128 display (should be 8 pixel aligned)
// maximum 255x255!
#define CHART_X 104
#define CHART_Y 0
#define CHART_W 192
#define CHART_H 128

uint16_t cursorX = 0;
uint8_t stepX = 4;

struct sampleType {
    uint8_t value [MAX_SAMPLED_DEVICES][MAX_SAMPLES];
};
sampleType sampledData;
uint8_t chartMaxDevices = 0;
uint8_t chartSampleIndex = 0;
uint8_t chartCursorY = 0;
float chartSampleScale = (float) CHART_H / (SAMPLE_MAX-SAMPLE_MIN);

void m_chart_drawBackground(uint16_t x,uint16_t y,uint16_t w,uint16_t h) {
    display.fillRect(x, y, w, h, GxEPD_WHITE);
    display.drawRect(x, y, w, h, GxEPD_BLACK);
}

void m_chart_init(void) {
    for (uint8_t device=0; device < MAX_SAMPLED_DEVICES; device++) {
        for (uint8_t sample=0; sample < MAX_SAMPLES; sample++) {
            sampledData.value[device][sample]=0;
        }
    }
    chartRefresh = millis() + 20000; // wait initially 20sec for packages to arrive
    Serial.println(F(" Drawing Chart Background"));
    // display.init();
    display.setFullWindow();
    m_chart_drawBackground(CHART_X,CHART_Y,CHART_W,CHART_H);
    display.displayWindow(CHART_X,CHART_Y,CHART_W,CHART_H);
}

bool m_chart_getLastSample(uint8_t deviceIndex, uint8_t* y1, uint8_t* y2) {
    if (chartSampleIndex<0) {
        return false;
    }
    *y1 = sampledData.value[deviceIndex][chartSampleIndex-1];
    *y2 = sampledData.value[deviceIndex][chartSampleIndex];
    Serial.print("m_chart_getLastSample: ");
    Serial.print(chartSampleIndex);
    Serial.print(" Y1:");
    Serial.print(*y1);
    Serial.print(" Y2:");
    Serial.println(*y2);
    return true;
}

/*!
   @brief    add a new sample to chart storage
    @param    propertyId    name of property
*/
void m_chart_addSample(String propertyId) {
    for (uint8_t deviceIndex = 0; deviceIndex < MAX_DEVICES; deviceIndex++) {
        for (uint8_t sensorIndex = 0; sensorIndex < MAX_SENSORS; sensorIndex++) {
            for (uint8_t propertyIndex = 0; propertyIndex < MAX_PROPERTIES; propertyIndex++) {
                if (dataStorage.devices[deviceIndex].sensors[sensorIndex].properties[propertyIndex].propertyId.startsWith(propertyId)) {
                    sampledData.value[deviceIndex][chartSampleIndex]=floor(dataStorage.devices[deviceIndex].sensors[sensorIndex].properties[propertyIndex].value * chartSampleScale);
                    if (deviceIndex>chartMaxDevices) chartMaxDevices=deviceIndex;
                    #ifdef SERIAL_TRACE
                        Serial.print(F(" Sampled: "));
                        Serial.print(sampledData.value[deviceIndex][chartSampleIndex]);
                        Serial.print(" (");
                        Serial.print(chartSampleIndex);
                        Serial.println(")");
                    #endif
                }
            }
        }
    }
}
/*!
   @brief    mover complete chart one sample to the left
*/
void m_ui_advanceSamples(void) {
    chartSampleIndex++;
    if (chartSampleIndex>MAX_SAMPLES-1) {
        m_chart_drawBackground(CHART_X,CHART_Y,CHART_W,CHART_H);        
        chartSampleIndex = MAX_SAMPLES-1;
        uint8_t y1=0, y2=0;
        for (uint8_t deviceIndex = 0; deviceIndex <= chartMaxDevices; deviceIndex++) {
            for (uint8_t sampleNumber = 0; sampleNumber<MAX_SAMPLES-1; sampleNumber++) {
                y1 = sampledData.value[deviceIndex][sampleNumber];
                y2 = sampledData.value[deviceIndex][sampleNumber+1];
                sampledData.value[deviceIndex][sampleNumber]=sampledData.value[deviceIndex][sampleNumber+1]; // shift all data by to left
                display.drawLine(CHART_X+sampleNumber+1,CHART_Y+CHART_H-y1-1,CHART_X+sampleNumber+2,CHART_Y+CHART_H-y2-1,GxEPD_BLACK);

            }
        }
    } else { // move cursor to the right until MAX_SAMPLES reached
        cursorX++;
    }
}

/*!
   @brief    draw chart and advance by one step (row)
*/
void m_ui_drawChart(void) {
    display.setFullWindow();
    uint8_t y1 = 0;
    uint8_t y2 = 0;
    for (uint8_t deviceIndex = 0; deviceIndex <= chartMaxDevices; deviceIndex++) {
        if (m_chart_getLastSample(deviceIndex, &y1, &y2)) {
            display.drawLine(CHART_X+cursorX+1,CHART_Y+CHART_H-y1-1,CHART_X+cursorX+2,CHART_Y+CHART_H-y2-1,GxEPD_BLACK);
        };
    }
    m_ui_advanceSamples();
    display.displayWindow(CHART_X,CHART_Y,CHART_W,CHART_H);
};

void m_chart_loop(void) {
    if (chartRefresh < millis()) {
        chartRefresh = millis() + CHART_REFRESH;
        #ifdef COMUNICATION_ESPNOW
            esp_now_unregister_recv_cb(); // stop esp-now callback during e-paper update because new packages interfeer the update process
        #endif
        m_ui_drawChart();
        #ifdef COMUNICATION_ESPNOW
            esp_now_register_recv_cb(c_espnow_dataReceived);  // re register esp-now callback
        #endif
    }
}