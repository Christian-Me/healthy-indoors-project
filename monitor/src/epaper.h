// GxEPD2_HelloWorld.ino by Jean-Marc Zingg

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
// #include <GFX.h>

#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h> // including both doesn't hurt
#include <GxEPD2_3C.h> // including both doesn't hurt

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/Org_01.h>
#include <Fonts/TomThumb.h>
#include <Fonts/Picopixel.h>

// copy constructor for your e-paper from GxEPD2_Example.ino, and for AVR needed #defines
#define MAX_DISPLAY_BUFFER_SIZE 5000 // 
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
// GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ SS, /*DC=4*/ 0, /*RST=5*/ 2, /*BUSY=16*/ 4));
GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=15*/ D8, /*DC=4*/ D3, /*RST=5*/ D4, /*BUSY=16*/ D2));

// BUSY -> D2 (GPIO04)  Busy
// RST  -> D4 (GPIO05)  Reset
// DC   -> D3 (GPIO04)  Data Command
// CS   -> D8 (GPIO15)  Chip Select
// CLK  -> D5 (GPIO)    Clock 
// DIN  -> D7 (GPIO)    Data IN
// GND  -> GND 3.3V     3.3V
// 296x128

const char HelloWorld[] = "Healthy Indoors Project";


void m_epaper_splash()
{
  display.setRotation(1);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(x, y);
  display.print(HelloWorld);
  display.display(true); // full update
}

// setup for e-paper display
void m_epaper_setup()
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