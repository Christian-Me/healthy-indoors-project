#include <Adafruit_NeoPixel.h>
#define PIN D1 // D4
#define LED_AMOUNT 8
#define LED_TYPE NEO_RGB
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#define NEOPIXEL_MIN_BRIGHTNESS 1          // min brightness of color indicator
#define NEOPIXEL_MAX_BRIGHTNESS 100         // max brightness of color indicator
#define NEOPIXEL_INDICATOR_BRIGHTNESS 150   // brightness of sensor indicator
#define NEOPIXEL_REFRESH 2000               // refresh rate;
#define NEOPIXEL_MAX_SAMPLED_DEVICES 5      // maximum device to sample
#define NEOPIXEL_INDICATOR_FADE_STEP 1      // step to fade each loop 
#define NEOPIXEL_INDICATOR_FADE_WAIT 10     // step to fade each loop
#define NEOPIXEL_SINGLE_LED_VALUE 1         // for singe LED show 1=max, 2=average, 3=min

struct inicatorSampleType {
    uint8_t LED;
    uint8_t value;
};
inicatorSampleType deviceIndicator [NEOPIXEL_MAX_SAMPLED_DEVICES];
unsigned long millisDelay = 0;
unsigned long neopixelRefresh = 0;


// easy way to convert neopixel packed color to rgbw values and back
union neopixelColor {           // 32 bit color
    uint32_t color;
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t w;
    } value;
};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, LED_TYPE);

/*!
   @brief    initialize neopixel
    @param    brightness    maximum brightness
*/
void m_neopixel_init (uint8_t brightness) {
    Serial.println(F("Initializing neopixel"));
    strip.begin();
    strip.setBrightness(brightness);
    strip.clear();
    strip.show(); // Initialize all pixels to 'off'
    Serial.print(F(" Number of LEDs: "));
    Serial.println(LED_AMOUNT);
    Serial.print(F(" GPIO: "));
    Serial.println(PIN);
    Serial.print(F(" LED Type: "));
    Serial.println(LED_TYPE);

    for (uint8_t i=0; i<NEOPIXEL_MAX_SAMPLED_DEVICES; i++) {
        deviceIndicator[i].LED=0;
        deviceIndicator[i].value=0;
    }
}
/*!
   @brief    show a singe value
    @param    led       number form 0 of the LED
    @param    value     float value
    @param    min       minimum range of value
    @param    max       maximum range of value
*/
void m_neopixel_showValue(uint16_t led, float value, float min, float max) {
    Serial.print("m_neopixel_showValue :");
    Serial.println(value);
    uint8_t brightness = floor(value*(NEOPIXEL_MAX_BRIGHTNESS-NEOPIXEL_MIN_BRIGHTNESS)/(max-min))+NEOPIXEL_MIN_BRIGHTNESS;
    uint16_t colorHue = floor(value*(65535/(max-min)));
    strip.setPixelColor(led,strip.ColorHSV(colorHue, 255, brightness));
}

float minValue = 0;
float maxValue = 0;
float maxLED = 0;

/*!
   @brief    callback function to set pixel for each sensor device
    @param    value     float value
    @param    index     index number of device
*/
void m_neopixel_setPixel(float value, int index) { // ToDo: Work for RGB chips
    if (dataStorage.getMaxDevices()==0) return;
    if (value>maxValue) value=maxValue;
    uint16_t colorHue = 0;
    uint16_t led = floor(value / ((maxValue-minValue) / LED_AMOUNT));
    uint8_t brightness = floor(value*(NEOPIXEL_MAX_BRIGHTNESS-NEOPIXEL_MIN_BRIGHTNESS)/(maxValue-minValue))+NEOPIXEL_MIN_BRIGHTNESS;
    #ifdef SERIAL_TRACE
        Serial.print(F(" m_neopixel_setPixel: led="));
        Serial.print(led);
        Serial.print(F(" brightness="));
        Serial.println(brightness);
    #endif
    for (uint16_t i = 0; i < LED_AMOUNT; i++) {
        if (i > led) break;
        colorHue = floor(i * (65535 / LED_AMOUNT));
        strip.setPixelColor(i,strip.ColorHSV(colorHue, 255, brightness));
    }
    if (led>maxLED) maxLED = led;
}

/*!
   @brief    refresh led / led strip
    @param    propertyId    name of the property
    @param    min       minimum range of value
    @param    max       maximum range of value
*/
void m_neopixel_refresh(String propertyId, float min, float max){
    if (neopixelRefresh > millis()) return;
    neopixelRefresh = millis() + NEOPIXEL_REFRESH;
    Serial.println("m_neopixel_refresh");
    minValue = min;
    maxValue = max;
    maxLED = 0;
    #if (LED_AMOUNT == 1)
        float min=99999, max=0, avg=0;
        if (dataStorage.getMinMaxAvg("iaq",&min,&max,&avg)) {
            switch (NEOPIXEL_SINGLE_LED_VALUE) {
                case 1:
                    m_neopixel_showValue(0,*max,0,500);
                    break;
                case 2:
                    m_neopixel_showValue(0,*avg,0,500);
                    break;
                case 3:
                    m_neopixel_showValue(0,*min,0,500);
                    break;
            }
        }
    #else
        dataStorage.forEachProperty(propertyId, &m_neopixel_setPixel);
        for (int16_t i = maxLED+1; i < LED_AMOUNT; i++) { // switch off LEDs after max LED
            strip.setPixelColor(i,0);
        }
    #endif
    strip.show();
}
/*!
   @brief    show indicator of last updating device
*/
void m_neopixel_showIndicator() {
    if (millisDelay > millis()) return;
    millisDelay = millis() + NEOPIXEL_INDICATOR_FADE_WAIT;
    if (dataStorage.getMaxDevices()==0) return;
    int value;
    neopixelColor currentColor;
    for (uint8_t i=0; i<dataStorage.getMaxDevices(); i++) {
        if (deviceIndicator[i].value>0) {
            value=deviceIndicator[i].value-NEOPIXEL_INDICATOR_FADE_STEP;
            if (value<=0) value=0;
            deviceIndicator[i].value=value;
            currentColor.color=strip.getPixelColor(deviceIndicator[i].LED);
            strip.setPixelColor(deviceIndicator[i].LED,currentColor.value.r,currentColor.value.g,currentColor.value.b,value);
            strip.show();
        };
    }
}
/*!
   @brief    neopixel loop function
*/
void m_neopixel_loop(void) {
    m_neopixel_refresh("iaq",0,500);
    #if (LED_AMOUNT!=1 && LED_TYPE==NEO_RGBW) // indicator only for more than one RGBW LEDs 
        m_neopixel_showIndicator();
    #endif
}
