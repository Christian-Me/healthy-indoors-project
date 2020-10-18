#ifndef EPAPER_DISPLAY_H__
#define EPAPER_DISPLAY_H__

// based on GxEPD2_HelloWorld.ino by Jean-Marc Zingg

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
// #include <GFX.h>

#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h> // including both doesn't hurt
#include <GxEPD2_3C.h> // including both doesn't hurt

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/Org_01.h>
#include <Fonts/TomThumb.h>
#include <Fonts/Picopixel.h>

#define MATRIX_BACKGROUND GxEPD_WHITE
#define MATRIX_FOREGROUND GxEPD_BLACK

// copy constructor for your e-paper from GxEPD2_Example.ino, and for AVR needed #defines
#define MAX_DISPLAY_BUFFER_SIZE 5000 // 
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
// GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ SS, /*DC=4*/ 0, /*RST=5*/ 2, /*BUSY=16*/ 4));
GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ D8, /*DC=4*/ D3, /*RST=5*/ D4, /*BUSY=16*/ D2));

class epaperDisplay {
  public:
    bool init(void);
    void loop(void);
};

// BUSY -> D2 (GPIO04)  Busy
// RST  -> D4 (GPIO05)  Reset
// DC   -> D3 (GPIO04)  Data Command
// CS   -> D8 (GPIO15)  Chip Select
// CLK  -> D5 (GPIO)    Clock 
// DIN  -> D7 (GPIO)    Data IN
// GND  -> GND 3.3V     3.3V
// 296x128

// setup for e-paper display
void epaperDisplay::init()
{
  display.init(115200);
  Serial.println(F("Setup e-paper"));
  m_epaper_splash();
  Serial.print(F(" Update mode: "));
  if (display.epd2.hasFastPartialUpdate) {
    Serial.println(F("fast partial mode"));
  }
  else if (display.epd2.hasPartialUpdate) {
    Serial.println(F("slow partial mode"));
  } else {
    Serial.println(F("no partial mode"));
  }
  delay(2000); // ESPs do not like delays - but before wifi starts it should be OK
}

void epaperDisplay::loop() {

}

#endif