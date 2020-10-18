#ifndef CHART_H__
#define CHART_H__


#include "color.h" // color manipulation tools
#include "monitor_callbacks.h" // callback functions for monitor styling
#include "chart_callbacks.h" // callback functions for chart styling

#define MAX_SAMPLED_DEVICES 5   // maximum device to sample
#define MAX_SAMPLES 190         // sample memory for selected values display width / 8
#define SAMPLE_MIN 0            // minimum of sampled values
#define SAMPLE_MAX 500          // maximum of sampled values
#define PIXEL_PER_SAMPLE 2      // 

/* chart window for 296 x 128 display (should be 8 pixel aligned)
 maximum 255x255!
#define chartX 104
#define chartY 0
#define chartW 192
#define chartH 128
*/

struct samplesType {
    uint8_t value [MAX_SAMPLED_DEVICES][MAX_SAMPLES];
};
struct sampleType {
    uint8_t value [MAX_SAMPLED_DEVICES];
};

class chart {
  private:
    String chartName;
    unsigned long chartRefresh = 0;
    unsigned long timeStart = 0;
    uint16_t chartPeriod;
    uint16_t cursorX = 0;
    uint8_t stepX = 4;
    samplesType sampledData;
    uint8_t chartMaxDevices = 0;
    uint8_t chartSampleIndex = 0;
    uint8_t chartCursorY = 0;
    bool chartVisible = false;
    void drawTitle(void);
    void drawBackground(void);
    void drawScale(void);
    bool getLastSample(uint8_t deviceIndex, uint8_t* y1, uint8_t* y2);
    bool getSamples(sampleType* actual, sampleType* previous, uint8_t* length, int index);
    void advanceSamples(void);
    void redrawChart();
    void drawChartSample(void);
    colorCallback getColor = nullptr;
    alarmCallback getAlarm = nullptr;
    viewportCallback drawGrid = nullptr;
    drawSampleCallback drawSample = nullptr;
    float chartSampleScale = 1;
    uint16_t chartX, chartY, chartW, chartH, chartDrawingW;
    uint16_t gridMainX, gridMinorX, gridMainY, gridMinorY;
  public:
    bool init(const __FlashStringHelper* , bool, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, int , int, colorCallback, viewportCallback, alarmCallback, drawSampleCallback);
    void setGrid(uint16_t mainX, uint16_t minorX, uint16_t mainY, uint16_t minorY);
    void loop(void);
    void addSample(String propertyId);
    void visible(bool visibility);
};

void chart::setGrid(uint16_t mainX, uint16_t minorX, uint16_t mainY, uint16_t minorY) {
    gridMainX=mainX;
    gridMinorX=minorX;
    gridMainY=mainY;
    gridMinorY=minorY;
};

void chart::visible(bool visibility) {
    chartVisible = visibility;
    drawBackground();
    drawScale();
    redrawChart();
    drawTitle();
};

void chart::drawTitle() {
    if (!chartVisible) return;
    display.setTextFont(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(chartX+5, chartY+10);
    display.print(chartName);
}

void chart::drawBackground() {
    if (!chartVisible) return;
    display.fillRect(chartX,chartY,chartW,chartH, MATRIX_BACKGROUND);
    #ifdef MONITOR_EPAPER // frame only for B&W displays
        display.drawRect(chartX,chartY,chartW,chartH, MATRIX_FOREGROUND);
    #endif
    display.setWindow(chartX,chartY,chartW,chartH);
    if (drawGrid!=nullptr) drawGrid(chartX,chartY,chartW,chartH);
    display.setWindow(0,0,display.width(),display.height());
}
/*!
   @brief   initialize chart
    @param    name          name of the chart diplayed as title
    @param    visibility    initial visibility of the chart. If invisible data will be recorded
    @param    x             top horizontal position
    @param    y             top vertical position
    @param    w             width of chart
    @param    h             hight of chart
    @param    valueMin      lowest value to expect
    @param    valueMax      highest value to expect
    @param    colorCallback (optional) callback function to determin the color depending of a value  
    @param    gridCallback  (optional) callback function to draw gridlines  
    @param    alarmCallback (optional) callback function to signal alarm state
    @param    drawCallback  (optional) callback function to draw the chart
*/
bool chart::init(const __FlashStringHelper *name, bool visibility, uint16_t period, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int valueMin, int valueMax, colorCallback getColorCallback=nullptr, viewportCallback getDrawGrid=nullptr, alarmCallback getAlarmCallback=nullptr, drawSampleCallback thisDrawSampleCallback=nullptr) {
    chartName = name;
    chartVisible = visibility;
    chartPeriod = period * 1000;
    timeStart = millis();
    chartX=x;
    chartY=y;
    chartW=w;
    chartH=h;
    chartDrawingW=w;
    chartSampleScale = (float) chartH / (valueMax-valueMin);
    getColor = getColorCallback;
    getAlarm = getAlarmCallback;
    drawGrid = getDrawGrid;
    drawSample = thisDrawSampleCallback;
    for (uint8_t device=0; device < MAX_SAMPLED_DEVICES; device++) {
        for (uint8_t sample=0; sample < MAX_SAMPLES; sample++) {
            sampledData.value[device][sample]=0;
        }
    }
    chartRefresh = millis() + chartPeriod; // wait initially 20sec for packages to arrive
    Serial.println(F(" Drawing Chart Background"));
    #ifdef MONITOR_EPAPER
        display.setFullWindow();
        drawBackground();
        display.displayWindow();
    #endif
    #ifdef MONITOR_TFT
        drawBackground();
        drawScale();
    #endif
    return true;
}

/*!
   @brief   get (descending) stored chart samples
    @param    actual      pointer to sampleType array of last reading
    @param    previous    pointer (or nullptr) to sampleType array of previous reading (usefull to draw line charts and only calling this function once)
    @param    length      amount of samples in array
    @param    index       (optional) if given it returns a specific reading
    @returns  updated arrays and length
*/
bool chart::getSamples(sampleType* actual, sampleType* previous, uint8_t* length, int index = -1) {
    if (index == -1) index = chartSampleIndex;
    *length = chartMaxDevices+(uint8_t)1;
    for (uint8_t device=0; device<*length; device++) {
        actual->value[device]=sampledData.value[device][index];
        if (previous!=nullptr && index>0) previous->value[device]=sampledData.value[device][index-1];
    }
    sortArrayReverse(actual->value, *length);
    if (previous!=nullptr) sortArrayReverse(previous->value, *length);
    return (length>0);
}

bool chart::getLastSample(uint8_t deviceIndex, uint8_t* y1, uint8_t* y2) {
    if (chartSampleIndex<0) return false;
    *y1 = sampledData.value[deviceIndex][chartSampleIndex-1];
    *y2 = sampledData.value[deviceIndex][chartSampleIndex];
    #ifdef SERIAL_TRACE
        Serial.print("m_chart_getLastSample: ");
        Serial.print(chartSampleIndex);
        Serial.print(" Y1:");
        Serial.print(*y1);
        Serial.print(" Y2:");
        Serial.println(*y2);
    #endif
    return true;
}

/*!
   @brief    add a new sample to chart storage form previos sensor readings
    @param    propertyId    name of property
*/
void chart::addSample(String propertyId) {
    float min=99999, max=0, avg=0;
    #ifdef SERAIL_TRACE
        Serial.print(F("Add sample: "));
        Serial.println(propertyId);
    #endif
    dataStorage.getMinMaxAvg(propertyId,&min,&max,&avg); // get sensor readings
    #ifdef MONITOR_TFT
        uint8_t alarmState = getAlarm(max);
        #ifdef SERIAL_TRACE
            Serial.print(F("Check if alarm level for "));
            Serial.print(propertyId);
            Serial.print(F(" value="));
            Serial.print(max);
            Serial.print(F(" state="));
            Serial.println(alarmState);
        #endif
        switch (alarmState) { // disable screensaver for all alarm levels >2
            case 3: matrixDisplay.disableScreensaver(20);
                    break;
            case 4: matrixDisplay.disableScreensaver(100);
                    break;
            case 5: matrixDisplay.disableScreensaver(150);
                    break;
            case 6: matrixDisplay.disableScreensaver(200);
                    break;
            case 7: matrixDisplay.disableScreensaver(255);
                    break;
            default: matrixDisplay.enableScreensaver();
        }
    #endif

    // loop through all data in dataStorange: device/sensor/property and add mathing values to chart
    for (uint8_t deviceIndex = 0; deviceIndex < MAX_DEVICES; deviceIndex++) {
        for (uint8_t sensorIndex = 0; sensorIndex < MAX_SENSORS; sensorIndex++) {
            for (uint8_t propertyIndex = 0; propertyIndex < MAX_PROPERTIES; propertyIndex++) {
                if (dataStorage.devices[deviceIndex].sensors[sensorIndex].properties[propertyIndex].propertyId.equals(propertyId)) {
                    if (dataStorage.devices[deviceIndex].sensors[sensorIndex].properties[propertyIndex].value!=UNSUPPORTED_VALUE) {
                        sampledData.value[deviceIndex][chartSampleIndex]=floor(dataStorage.devices[deviceIndex].sensors[sensorIndex].properties[propertyIndex].value * chartSampleScale);
                        if (deviceIndex>chartMaxDevices) chartMaxDevices=deviceIndex;
                        #ifdef SERIAL_TRACE
                            Serial.print(F(" Sampled: "));
                            Serial.print(propertyId);
                            Serial.print("=");
                            Serial.print(sampledData.value[deviceIndex][chartSampleIndex]);
                            Serial.print(" (#");
                            Serial.print(chartSampleIndex);
                            Serial.println(")");
                        #endif
                    }
                }
            }
        }
    }
}

void chart::redrawChart() {
    #ifdef MONITOR_EPAPER
        if (chartVisible) drawBackground(chartX,chartY,chartDrawingW,chartH);        
        uint8_t y1=0, y2=0;
        for (uint8_t deviceIndex = 0; deviceIndex <= chartMaxDevices; deviceIndex++) {
            for (uint8_t sample = 0; sample<MAX_SAMPLES-1; sample++) {
                sampledData.value[deviceIndex][sample]=sampledData.value[deviceIndex][sample+1]; // shift all data by to left
                if (chartVisible) {
                    y1 = sampledData.value[deviceIndex][sample];
                    y2 = sampledData.value[deviceIndex][sample+1];
                    display.drawLine(chartX+sample+1,chartY+chartH-y1-1,chartX+sample+2,chartY+chartH-y2-1,MATRIX_FOREGROUND);
                    drawSample(index,sample,chartX,chartY,chartW,chartH,0,actual.value[sample],chartSampleScale, color32to16(color));
                }
            }
    #endif
    #ifdef MONITOR_TFT
        sampleType actual,last;
        uint8_t length;
        packedColor color;
        if (chartVisible) { // clear viewport indicated by sample = -1
            drawSample(0, -1,chartX,chartY,chartDrawingW,chartH,0,0,0,0);
        }
        for (uint8_t index = 0; index <= chartSampleIndex; index++) {
            getSamples(&actual,&last,&length,index);
            // we have to draw all sample (sorted) as some smaller values will be overwritten
            for (uint8_t sample=0; sample<length; sample++) {
                if (chartVisible) {
                    color.color=getColor(actual.value[sample]/chartSampleScale);
                    drawSample(index,sample,chartX,chartY,chartDrawingW,chartH,last.value[sample],actual.value[sample],chartSampleScale, color32to16(color));
                }
                sampledData.value[sample][index]=sampledData.value[sample][index+1]; // shift all data by to left, could be done more effective by ring buffer but fine on ESPs
            }
        }
    #endif
}
/*!
   @brief    advance chart by one and move complete chart one sample to the left if necessary
*/
void chart::advanceSamples(void) {
    chartSampleIndex++;
    if (chartSampleIndex>chartDrawingW-3 || chartSampleIndex>MAX_SAMPLES-1) { // right edge of chart or maximum samples reached
        chartSampleIndex = (chartDrawingW<MAX_SAMPLES) ? chartDrawingW-1 : MAX_SAMPLES-1;
        redrawChart();
    } else { // move cursor to the right until MAX_SAMPLES reached
        cursorX++;
    }
}
/*!
   @brief    draw side scale on the chart
*/
void chart::drawScale(void) {
    if (!chartVisible) return;
    #ifdef MONITOR_TFT
        chartDrawingW = chartW - 10; // this reduce the available drawing area
        display.drawRect(chartX+chartW-10, chartY, 10, chartH, MATRIX_FOREGROUND);
        packedColor color;
        for (uint8_t y = 2; y<chartH; y++) {
            color.color=getColor(y/chartSampleScale);
            display.drawFastHLine(chartX+chartW-9,chartY+chartH-y,8,color32to16(color));
        }
    #endif
}
/*!
   @brief    draw chart and advance by one step (row)
*/
void chart::drawChartSample(void) {
    #ifdef MONITOR_EPAPER
        if (chartVisible) {
            display.setFullWindow();
            uint8_t y1 = 0;
            uint8_t y2 = 0;
            for (uint8_t deviceIndex = 0; deviceIndex <= chartMaxDevices; deviceIndex++) {
                if (getLastSample(deviceIndex, &y1, &y2)) {
                        display.drawLine(chartX+cursorX+1,chartY+chartH-y1-1,chartX+cursorX+2,chartY+chartH-y2-1,MATRIX_FOREGROUND);
                }
            }
        }
    #endif
    #ifdef MONITOR_TFT
        if (chartVisible) {
            sampleType actual, last;
            uint8_t length;
            packedColor color;
            getSamples(&actual,&last,&length);
            // we have to draw all sample (sorted) as some smaller values will be overwritten
            for (uint8_t sample=0; sample<length; sample++) {
                color.color=getColor(actual.value[sample]/chartSampleScale);
//                #ifdef SERIAL_TRACE
                    Serial.print(F(" draw chart x="));
                    Serial.print(cursorX);
                    Serial.print(F(" y1="));
                    Serial.print(last.value[sample]);
                    Serial.print(F(" y2="));
                    Serial.print(actual.value[sample]);
                    Serial.print(F(" value="));
                    Serial.print(actual.value[sample]/chartSampleScale);
                    Serial.print(F(" r="));
                    Serial.print(color.value.r);
                    Serial.print(F(" g="));
                    Serial.print(color.value.g);
                    Serial.print(F(" b="));
                    Serial.println(color.value.b);
//                #endif
                drawSample(cursorX,sample,chartX,chartY,chartDrawingW,chartH,last.value[sample],actual.value[sample],chartSampleScale, color32to16(color));
                // display.drawFastVLine(chartX+cursorX+1,chartY+chartH-actual.value[sample]-1,actual.value[sample]-1,color32to16(color));
            }
            // then the grid
            if (drawGrid!=nullptr) drawGrid(chartX,chartY,chartW,chartH);
        }
    #endif
    advanceSamples();
    drawTitle();
    #ifdef MONITOR_EPAPER
        if (chartVisible) display.displayWindow(chartX,chartY,chartW,chartH);
    #endif
};

void chart::loop(void) {
    if (chartRefresh < millis()) {
        chartRefresh = millis() + chartPeriod;
        comm1.pauseReceive(); // stop esp-now callback during e-paper update because new packages interfeer the update process
        drawChartSample();
        // comm1.resumeReceive(); // re register esp-now callback
    }
}

#endif