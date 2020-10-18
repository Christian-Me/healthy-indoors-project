#include <Adafruit_NeoPixel.h>
#include "color.h"
#include "monitor_callbacks.h"

// Neopixel types
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
#define NEOPIXEL_MAX_SAMPLED_DEVICES 10     // maximum device to sample
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


class m_neopixel{
  private:
    Adafruit_NeoPixel strip;
    bool initialized = false;
    unsigned long timeout = 0;
    uint16_t neopixelAmount = 0;
    uint16_t neopixelType = NEO_GRB;
    String neopixelProperty = "iaq";
    String output = "";
    void setPixel(float value, int index);
    void refresh(String propertyId, float min, float max);
    void showIndicator();
  public:
    m_neopixel();
    void showValue(uint16_t led, float value, float min, float max);
    void init(uint8_t pin, String property, uint16_t amount, uint16_t type, uint8_t brightness);
    void loop();
};

/*!
   @brief    constructor of neopixel object
    @
*/
m_neopixel::m_neopixel() {
}

/*!
   @brief    initialize neopixel
    @param      pin             GPIO pin number of neopixel bus
    @param      property        (optional) property to show defaults="iaq"
    @param      amount          (optional) amount of led chips on bus defaults=1
    @param      type            (optional) type of LED chips defaults=RGB
    @param      brightness      (optional) maximum brightness defaults=128
*/
void m_neopixel::init (uint8_t pin, String property = "iaq", uint16_t amount = 1 , uint16_t type = NEO_RGBW, uint8_t brightness = 128) {
    Serial.println(F("Initializing neopixel"));
    strip.begin();
    Serial.print(F(" Property: "));
    Serial.println(property);
    neopixelProperty = property;
    Serial.print(F(" Number of LEDs: "));
    Serial.println(amount);
    neopixelAmount=amount;
    strip.updateLength(amount);
    Serial.print(F(" GPIO: "));
    Serial.println(pin);
    strip.setPin(pin);
    Serial.print(F(" LED Type: "));
    Serial.println(type);
    strip.updateType(type);
    Serial.print(F(" Brightness: "));
    Serial.println(brightness);
    strip.setBrightness(brightness);
    strip.clear();
    strip.show(); // Initialize all pixels to 'off'

    neopixelColor color;
    for (float value=0; value < 500; value++) {
        Serial.print(value);
        color.color=getIAQColor(value);
        Serial.printf(" r:%d g:%d b:%d ",color.value.r, color.value.g, color.value.b);
        Serial.println();
        strip.setPixelColor(0,color.value.r, color.value.g, color.value.b);
        strip.show();
        delay(10);
    }

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
void m_neopixel::showValue(uint16_t led, float value, float min, float max) {
    Serial.print("m_neopixel_showValue :");
    Serial.print(value);
    neopixelColor color;
    color.color=getIAQColor(value);
    Serial.printf(" r:%d g:%d b:%d ",color.value.r, color.value.g, color.value.b);
    Serial.println();
    strip.setPixelColor(led,color.value.r,color.value.g,color.value.b);
    /*
    uint8_t brightness = floor(value*(NEOPIXEL_MAX_BRIGHTNESS-NEOPIXEL_MIN_BRIGHTNESS)/(max-min))+NEOPIXEL_MIN_BRIGHTNESS;
    uint16_t colorHue = floor(value*(65535/(max-min)));
    strip.setPixelColor(led,strip.ColorHSV(colorHue, 255, brightness));
    */
}

float minValue = 0;
float maxValue = 0;
float maxLED = 0;

/*!
   @brief    callback function to set pixel for each sensor device
    @param    value     float value
    @param    index     index number of device
*/
void m_neopixel::setPixel(float value, int index) { // ToDo: Work for RGB chips
    
    if (dataStorage.getMaxDevices()==0) return;
    if (value>maxValue) value=maxValue;

    uint16_t colorHue = 0;
    uint16_t led = floor(value / ((maxValue-minValue) / neopixelAmount));
    uint8_t brightness = floor(value*(NEOPIXEL_MAX_BRIGHTNESS-NEOPIXEL_MIN_BRIGHTNESS)/(maxValue-minValue))+NEOPIXEL_MIN_BRIGHTNESS;
    for (uint16_t i = 0; i < neopixelAmount; i++) {
        if (i > led) break;
        colorHue = floor(i * (65535 / neopixelAmount));
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
void m_neopixel::refresh(String propertyId, float minValue, float maxValue){
    if (neopixelRefresh > millis()) return;
    neopixelRefresh = millis() + NEOPIXEL_REFRESH;
    Serial.println("m_neopixel_refresh");
    maxLED = 0;
    if (neopixelAmount == 1) {
        float min=99999, max=0, avg=0;
        if (dataStorage.getMinMaxAvg("iaq",&min,&max,&avg)) {
            switch (NEOPIXEL_SINGLE_LED_VALUE) {
                case 1:
                    showValue(0,max,minValue,maxValue);
                    break;
                case 2:
                    showValue(0,avg,minValue,maxValue);
                    break;
                case 3:
                    showValue(0,min,minValue,maxValue);
                    break;
            }
        }
    } else {
        // dataStorage.forEachProperty(propertyId, &m_neopixel::setPixel(strip, neopixelAmount)); // this is not working as I could not manage pointers to class members :(
        uint8_t index = 0;
        for (uint8_t device=0; device < MAX_DEVICES; device++) {
            for (uint8_t sensor=0; sensor < MAX_SENSORS; sensor++) {
                for (uint8_t property=0; property < MAX_PROPERTIES; property++) {
                    if (dataStorage.devices[device].sensors[sensor].properties[property].propertyId == propertyId) {
                        setPixel(dataStorage.devices[device].sensors[sensor].properties[property].value,index);
                        index++;
                    };
                }
            }
        }
    }
    for (int16_t i = maxLED+1; i < neopixelAmount; i++) { // switch off LEDs after max LED
        strip.setPixelColor(i,0);
    }
    strip.show();
}
/*!
   @brief    show indicator of last updating device
*/
void m_neopixel::showIndicator() {
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
void m_neopixel::loop(void) {
    refresh("iaq",0,500);
    if (neopixelAmount!=1 && neopixelType==NEO_RGBW){ // indicator only for more than one RGBW LEDs 
        showIndicator();
    }
}
