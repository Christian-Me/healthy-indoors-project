#define UI_GRID_X 8
#define UI_GRID_Y 8
#define UI_REFRESH 30000 // refresh every 30sec
unsigned long uiRefresh = 0;

//
uint16_t cx(uint8_t x) {
    return x*UI_GRID_X;
}
uint16_t cy(uint8_t y) {
    return y*UI_GRID_X;
}

/*!
   @brief    print a float value at a fixed decimal point
    @param    x         x coordinate
    @param    y         y coordinate
    @param    value     float value
    @param    dec       devimal places (default 1)
*/
void m_ui_printValue(uint8_t x, uint8_t y, float value, uint8_t dec = 1) {
    char valueString[18];
    int floorValue = floor(value);
    int fractionValue = floor((value-floorValue)*(10*dec));
    int16_t tbx, tby; uint16_t tbw, tbh;
    snprintf(valueString, sizeof(valueString), "%d", floorValue);
    display.getTextBounds(valueString, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor(x-tbw, y);
    display.print(valueString);
    display.setCursor(x+2, y);
    display.print(".");
    display.print(fractionValue);
}

/*!
   @brief    draw a numeric display area
    @param    uY        x coordinate in grid units
    @param    y         y coordinate
    @param    value     float value
    @param    dec       devimal places (default 1)
*/
void m_ui_drawNumElement(uint8_t uX, uint8_t uY, uint8_t uW, uint8_t uH, float value, float avg, float min, uint8_t width, uint8_t dec, const __FlashStringHelper *label,  const char* unit) {
//    display.setFullWindow();
    display.fillRect(cx(uX), cy(uY), cx(uW), cy(uH), GxEPD_WHITE);
    display.setFont(&FreeSans9pt7b);
    m_ui_printValue(cx(uX)+40, cy(uY)+21, value, 1);
    display.setFont(&Org_01);
    display.setCursor(display.getCursorX()+2, display.getCursorY());
    display.print(unit);
    display.setCursor(cx(uX)+1, cy(uY)+6);
    display.print(label);
    m_ui_printValue(cx(uX)+cx(uW)-11, cy(uY)+10, min, 1);
    m_ui_printValue(cx(uX)+cx(uW)-11, cy(uY)+20, avg, 1);
}

// draw a UI area
void m_ui_drawUiElements() {
    float min=99999, max=0, avg=0;
    bool wasUpdated = false;
    Serial.println(F("Update ui"));
    display.setFullWindow();
    if (dataStorage.getMinMaxAvg("iaq",&min,&max,&avg)) {
        Serial.print(F(" update IAQ :"));
        Serial.println(max);
        m_ui_drawNumElement(0,0,13,3,max,min,avg,6,1,F("IAQ"),"");
        wasUpdated=true;
    } else Serial.println(F("no iaq data available"));
    if (dataStorage.getMinMaxAvg("co2Equivalent",&min,&max,&avg)) {
        Serial.print(F(" update co2Equivalent :"));
        Serial.println(max);
        m_ui_drawNumElement(0,3,13,3,max,min,avg,6,1,F("CO2 eq"),"ppm");
        wasUpdated=true;
    } else Serial.println(F("no co2Equivalent data available"));
    if (dataStorage.getMinMaxAvg("breathVocEquivalent",&min,&max,&avg)) {
        Serial.print(F(" update breathVocEquivalent :"));
        Serial.println(max);
        m_ui_drawNumElement(0,6,13,3,max,min,avg,6,1,F("breath VOC"),"");
        wasUpdated=true;
    } else Serial.println(F("no breathVocEquivalent data available"));
    if (dataStorage.getMinMaxAvg("temperature",&min,&max,&avg)) {
        Serial.print(F(" update temperature: "));
        Serial.println(max);
        m_ui_drawNumElement(0,9,13,3,max,min,avg,6,2,F("Temperature"),"C");
        wasUpdated=true;
    } else Serial.println(F("no temperature data available"));
    if (dataStorage.getMinMaxAvg("humidity",&min,&max,&avg)) {
        Serial.print(F(" update humidity: "));
        Serial.println(max);
        m_ui_drawNumElement(0,12,13,3,max,min,avg,6,2,F("Humidity"),"%");
        wasUpdated=true;
    } else Serial.println(F("no humidity data available"));
    if (wasUpdated) display.display(false); // false = full update
    // display.displayWindow(0, 0, 192, 128);
    Serial.println(F("ui updated"));
}

void m_ui_init(void) {
    Serial.println(F("Init UI"));
    m_ui_drawUiElements();
    uiRefresh = millis() + 2000;
}

void m_ui_loop(void) {
    if (uiRefresh < millis()) {
        uiRefresh = millis() + UI_REFRESH;
        #ifdef COMUNICATION_ESPNOW
            esp_now_unregister_recv_cb(); // stop esp-now callback during e-paper update because new packages interfeer the update process
        #endif
        m_ui_drawUiElements();
        #ifdef COMUNICATION_ESPNOW
            esp_now_register_recv_cb(c_espnow_dataReceived);  // re register esp-now callback
        #endif
    }
}