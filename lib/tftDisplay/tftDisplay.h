#ifndef TFT_DISPLAY_H__
#define TFT_DISPLAY_H__

/*
 Adapted from the Adafruit graphicstest sketch.

 This sketch uses the GLCD font (font 1) only. Disable other fonts to make
 the sketch fit in an UNO!

 Make sure all the display driver and pin comnenctions are correct by
 editting the User_Setup.h file in the TFT_eSPI library folder.

 Note that yield() or delay(0) must be called in long duration for/while
 loops to stop the ESP8266 watchdog triggering.

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################
 */
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

// buttons
#define BUTTON1_PIN  35
#define BUTTON2_PIN  0

// LED Backlight dimming
// DO NOT CONNECT LED Pin to a GPIO PIN directly. Make sure a transistor switches the high current of the LED!
#define BACKLIGHT_PIN 4           // GPIO04 on TTGO-T-DISPLAY by LILYGO (transistor present)
#define BACKLIGHT_BRIGHTNESS 128  // default brightness
#define SCREENSAVER_TIMEOUT 30    // Screensaver timeout in seconds

// setting PWM properties
#define PWM_FREQUENCY  5000
#define PWM_CHANNEL    0
#define PWM_RESOLUTION 8

// define colors according to display capabilities
#ifdef MONITOR_EPAPER
  #define MATRIX_BACKGROUND TFT_BLACK
#endif
#ifdef MONITOR_TFT
  #define MATRIX_BACKGROUND TFT_BLACK
#endif
#define MATRIX_FOREGROUND TFT_WHITE

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>


TFT_eSPI display = TFT_eSPI();       // Invoke custom library
bool stateButton1 = false;
bool stateButton2 = false;

class tftDisplay {
    bool screensaverEnabled = true;
    uint8_t backlightBrightness = BACKLIGHT_BRIGHTNESS;
    uint8_t backlightDimmer = 128; // 50% dimming level
    uint8_t backlightState = 0;
    uint16_t screenTimeout = SCREENSAVER_TIMEOUT; // 0=no screensaver
    buttonCallback buttonEvent = nullptr; //callback for button events
    unsigned long screenTimer = 0;
    unsigned long dimmerTimer = 0;
    void backlightLoop(void);
  public:
    void setBacklight(uint8_t brightness);
    uint8_t getBacklight();
    void resetScreensaver(uint8_t time);
    void disableScreensaver(uint8_t brightness);
    void enableScreensaver(void);
    void registerButtonCallback(buttonCallback callback);
    bool init(void);
    void loop(void);
};

void IRAM_ATTR button1Pressed() {
    stateButton1=true;
}
void IRAM_ATTR button2Pressed() {
    stateButton2=true;
}

bool tftDisplay::init(void) {
  Serial.println(F("Initialize TFTDisplay ST7735"));
  // configure LED PWM functionalities
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  // attach the channel to the GPIO to be controlled
  Serial.print(F(" Backlight : "));
  Serial.println(BACKLIGHT_PIN);
  ledcAttachPin(BACKLIGHT_PIN, PWM_CHANNEL);
  Serial.print(F(" Button 1  : "));
  Serial.println(BUTTON1_PIN);
  attachInterrupt(BUTTON1_PIN, button1Pressed, FALLING);
  Serial.print(F(" Button 2  : "));
  Serial.println(BUTTON2_PIN);
  attachInterrupt(BUTTON2_PIN, button2Pressed, FALLING);
  screenTimer=millis()+screenTimeout*1000;
  // Use this initializer if you're using a 1.8" display
  display.init();   // initialize a ST7735S chip
  display.setRotation(1);
  
  Serial.print(F(" done! ("));
  uint16_t time = millis();
  display.fillScreen(TFT_BLACK);
  time = millis() - time;
  Serial.print(time, DEC);
  Serial.println("ms)");
  return true;
}

void tftDisplay::registerButtonCallback(buttonCallback callback) {
  buttonEvent = callback;
};

// loop for non breaking diming
void tftDisplay::backlightLoop(void) {
  if (millis()>dimmerTimer) {
    if(backlightDimmer>backlightState) {
      backlightState++;
      ledcWrite(PWM_CHANNEL, backlightState);
      dimmerTimer=millis()+20;
    }
    if(backlightDimmer<backlightState) {
      backlightState--;
      ledcWrite(PWM_CHANNEL, backlightState);
      dimmerTimer=millis()+100;
    }
  }
}
/*!
   @brief    set the brightness of the TFT backlight
    @param    brightness  brightness 0-255
*/
void tftDisplay::setBacklight(uint8_t brightness) {
  #ifdef SERIAL_TRACE
    Serial.print(F("Screensaver brightness="));
    Serial.println(brightness);
  #endif
  backlightDimmer = brightness;
}
/*!
   @brief    get the brightness of the TFT backlight
    @returns    brightness  brightness 0-255
*/
uint8_t tftDisplay::getBacklight() {
  return backlightDimmer;
}
/*!
   @brief    resets the screensaver timer
    @param    time  (optional) new timeout in seconds
*/
void tftDisplay::resetScreensaver(uint8_t time = SCREENSAVER_TIMEOUT){
  #ifdef SERIAL_TRACE
    Serial.println(F("Screensaver reset!"));
  #endif
  screenTimer=millis()+time*1000;
}
/*!
   @brief    disables the screensaver
    @param    brightness  (optional) new brightness
*/
void tftDisplay::disableScreensaver(uint8_t brightness = 0){
  #ifdef SERIAL_TRACE
    Serial.print(F("Screensaver disabled! "));
  #endif
  screensaverEnabled= false;
  if (brightness == 0) {
    setBacklight(BACKLIGHT_BRIGHTNESS);
  } else {
    setBacklight(brightness);
  }
};
/*!
   @brief    enables the screensaver
*/
void tftDisplay::enableScreensaver(void){
  if (!screensaverEnabled) {
    #ifdef SERIAL_TRACE
      Serial.println(F("Screensaver enabled!"));
    #endif
    screensaverEnabled= true;
    // setBacklight(backlightBrightness);
    resetScreensaver();
  }
};
/*!
   @brief    call this loop during main loop to enable screensaver and registered callbacks
*/
void tftDisplay::loop() {
  backlightLoop();
  if (stateButton1) {
    Serial.println(F("Button 1 pressed!"));
    stateButton1=false;
    if (buttonEvent!=nullptr) buttonEvent(1,1);
  }
  if (stateButton2) {
    Serial.println(F("Button 2 pressed!"));
    stateButton2=false;
    if (buttonEvent!=nullptr) buttonEvent(1,1);
  }
  if (screensaverEnabled && screenTimer>0 && millis()>screenTimer) { // check if it is time to start the screensaver
    Serial.println(F("start screensaver ..."));
    screenTimer=0;
    setBacklight(0);
  }
}

#endif