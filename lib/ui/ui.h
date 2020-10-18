#ifndef UI_H__
#define UI_H__

// #include "utils.h"
#include "monitor_callbacks.h" // callback functions for chart styling

#ifdef MONITOR_EPAPER
    #define UI_GRID_X 8
    #define UI_GRID_Y 8
#endif
#ifdef MONITOR_TFT
    #define UI_GRID_X 8
    #define UI_GRID_Y 9
#endif

#define UI_REFRESH 30000 // refresh every 30sec
unsigned long uiRefresh = 0;

//
uint16_t cx(uint8_t x) {
    return x*UI_GRID_X;
}
uint16_t cy(uint8_t y) {
    return y*UI_GRID_Y;
}

const char HelloWorld[] = "Healthy Indoors Project";

class ui {
  private:
    void printValue(uint8_t x, uint8_t y, float value, uint8_t dec);
    void drawNumElement(uint8_t uX, uint8_t uY, uint8_t uW, uint8_t uH, float value, float avg, float min, uint8_t width, uint8_t dec, const __FlashStringHelper *label,  const char* unit, colorCallback);
    void drawUiElements();
  public:
    void splash(void);
    bool init(void);
    void loop(void);
};

void printTextCentered(uint16_t x, uint16_t y, const __FlashStringHelper *label) {
    display.setCursor(x-display.textWidth(label)/2,y);
    display.print(label);
}

void ui::splash()
{
  display.setRotation(1);
  display.setTextColor(MATRIX_FOREGROUND);
  #ifdef MONITOR_EPAPER
    uint16_t tbw, tbh;
    int16_t tbx, tby; 
    display.setFont(&FreeSans9pt7b);
    display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center bounding box by transposition of origin:
    uint16_t x = ((display.width() - tbw) / 2); // - tbx;
    uint16_t y = ((display.height() - tbh) / 2); // - tby;
    display.setFullWindow();
  #endif
  display.fillScreen(MATRIX_BACKGROUND);
  #ifdef MONITOR_TFT
    display.setTextFont(4);
    printTextCentered(display.width()/2,display.height()/2-20,F("Healthy Indoors"));
    printTextCentered(display.width()/2,display.height()/2+20,F("Project"));
  #endif
  #ifdef MONITOR_EPAPER
    display.setCursor(x, y);
    display.print(HelloWorld);
    display.display(true); // full update
  #endif
}

/*!
   @brief    print a float value at a fixed decimal point
    @param    x         x coordinate
    @param    y         y coordinate
    @param    value     float value
    @param    dec       devimal places (default 1)
*/
void ui::printValue(uint8_t x, uint8_t y, float value, uint8_t dec = 1) {
    char valueString[18];
    int floorValue = floor(value);
    int fractionValue = floor((value-floorValue)*(10*dec));
    snprintf(valueString, sizeof(valueString), "%d", floorValue);
    uint16_t tbw;
    #ifdef MONITOR_EPAPER
        uint16_t tbh;
        int16_t tbx, tby; 
        display.getTextBounds(valueString, 0, 0, &tbx, &tby, &tbw, &tbh);
    #endif
    #ifdef MONITOR_TFT
        tbw = display.textWidth(valueString);
    #endif
    display.setCursor(x-tbw, y);
    display.print(valueString);
    display.setCursor(x+2, y);
    display.print(".");
    display.print(fractionValue);
}

/*!
   @brief    draw a numeric display area
    @param    uX        x coordinate in grid units
    @param    uy        y coordinate
    @param    value     float value
    @param    dec       devimal places (default 1)
*/
void ui::drawNumElement(uint8_t uX, uint8_t uY, uint8_t uW, uint8_t uH, float value, float avg, float min, uint8_t width, uint8_t dec, const __FlashStringHelper *label,  const char* unit, colorCallback getColor) {
//    display.setFullWindow();
    #ifdef MONITOR_EPAPER
        display.fillRect(cx(uX), cy(uY), cx(uW)-1, cy(uH), TFT_RED);
        display.setFont(&FreeSans9pt7b);
        printValue(cx(uX)+40, cy(uY)+21, value, 1);
        display.setFont(&Org_01);
        display.setCursor(display.getCursorX()+2, display.getCursorY());
        display.print(unit);
        display.setCursor(cx(uX)+1, cy(uY)+6);
        display.print(label);
        printValue(cx(uX)+cx(uW)-11, cy(uY)+10, min, 1);
        printValue(cx(uX)+cx(uW)-11, cy(uY)+20, avg, 1);
    #endif
    #ifdef MONITOR_TFT
        packedColor color;
        color.color=getColor(value);
        if (getLuma(color.color)>128) {
            display.setTextColor(TFT_BLACK);
        } else {
            display.setTextColor(TFT_WHITE);
        }
        display.fillRect(cx(uX), cy(uY), cx(uW)-1, cy(uH)-1, color32to16(color));
        display.setTextFont(1);
        printValue(cx(uX)+30, cy(uY)+16, value, 1);
        display.setTextFont(1);
        display.setCursor(display.getCursorX()+2, display.getCursorY());
        display.print(unit);
        display.setCursor(cx(uX)+1, cy(uY)+5);
        display.print(label);
        printValue(cx(uX)+cx(uW)-14, cy(uY)+5, min, 1);
        printValue(cx(uX)+cx(uW)-14, cy(uY)+16, avg, 1);
    #endif
}

// draw a UI area
void ui::drawUiElements() {
    float min=99999, max=0, avg=0;
    bool wasUpdated = false;
    Serial.println(F("Update ui"));
    #ifdef MONITOR_EPAPER
        display.setFullWindow();
    #endif
    if (dataStorage.getMinMaxAvg("iaq",&min,&max,&avg)) {
        Serial.print(F(" update IAQ :"));
        Serial.println(max);
        drawNumElement(0,0,13,3,max,min,avg,6,1,F("IAQ"),"", &getIAQColor);
        wasUpdated=true;
    } else Serial.println(F("no iaq data available"));
    if (dataStorage.getMinMaxAvg("co2Equivalent",&min,&max,&avg)) {
        Serial.print(F(" update co2Equivalent :"));
        Serial.println(max);
        drawNumElement(0,3,13,3,max,min,avg,6,1,F("CO2 eq"),"ppm", &getCO2Color);
        wasUpdated=true;
    } else Serial.println(F("no co2Equivalent data available"));
    if (dataStorage.getMinMaxAvg("breathVocEquivalent",&min,&max,&avg)) {
        Serial.print(F(" update breathVocEquivalent :"));
        Serial.println(max);
        drawNumElement(0,6,13,3,max,min,avg,6,1,F("breath VOC"),"", &getVOCColor);
        wasUpdated=true;
    } else Serial.println(F("no breathVocEquivalent data available"));
    if (dataStorage.getMinMaxAvg("temperature",&min,&max,&avg)) {
        Serial.print(F(" update temperature: "));
        Serial.println(max);
        drawNumElement(0,9,13,3,max,min,avg,6,2,F("Temperature"),"C", &getTemperatureColor);
        wasUpdated=true;
    } else Serial.println(F("no temperature data available"));
    if (dataStorage.getMinMaxAvg("humidity",&min,&max,&avg)) {
        Serial.print(F(" update humidity: "));
        Serial.println(max);
        drawNumElement(0,12,13,3,max,min,avg,6,2,F("Humidity"),"%", &getHumidityColor);
        wasUpdated=true;
    } else Serial.println(F("no humidity data available"));
    #ifdef MONITOR_EPAPER
        if (wasUpdated) display.display(false); // false = full update
        // display.displayWindow(0, 0, 192, 128);
    #endif
    Serial.print(F("ui "));
    if (wasUpdated) {
        Serial.println(F("updated"));
    } else {
        Serial.println(F("not updated"));

    }
}

bool ui::init(void) {
    Serial.println(F("Init UI"));
    drawUiElements();
    uiRefresh = millis() + 2000;
    return true;
}

void ui::loop(void) {
    if (uiRefresh < millis()) {
        uiRefresh = millis() + UI_REFRESH;
        #ifdef COMUNICATION_ESPNOW
            // esp_now_unregister_recv_cb(); // stop esp-now callback during e-paper update because new packages interfeer the update process
            comm1.pauseReceive();
        #endif
        drawUiElements();
        #ifdef COMUNICATION_ESPNOW
            // esp_now_register_recv_cb(c_espnow_dataReceived);  // re register esp-now callback
            // comm1.resumeReceive();
        #endif
    }
}

#endif