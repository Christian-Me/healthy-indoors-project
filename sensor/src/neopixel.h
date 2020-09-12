// #include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoPixel.h>

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
neopixelColor currentColor;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, NEOPIXEL_PIN, LED_TYPE);

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
    Serial.println(NEOPIXEL_PIN);
    Serial.print(F(" LED Type: "));
    Serial.println(LED_TYPE);
}

/*!
   @brief    show a singe value
    @param    led       number form 0 of the LED
    @param    value     float value
    @param    min       minimum range of value
    @param    max       maximum range of value
*/
void m_neopixel_showValue(uint16_t led, float value, float min, float max) {
    Serial.print(F("m_neopixel_showValue :"));
    Serial.print(value);
    uint8_t brightness = floor(value*(NEOPIXEL_MAX_BRIGHTNESS-NEOPIXEL_MIN_BRIGHTNESS)/(max-min))+NEOPIXEL_MIN_BRIGHTNESS;
    uint16_t colorHue = floor(value*(65535/(max-min)));
    Serial.print(F(" LED="));
    Serial.print(led);
    Serial.print(F(" hue="));
    Serial.print(colorHue);
    Serial.print(F(" brightness="));
    Serial.print(brightness);
    currentColor.color=strip.ColorHSV(colorHue, 255, brightness);
    Serial.print(F(" r="));
    Serial.print(currentColor.value.r);
    Serial.print(F(" g="));
    Serial.print(currentColor.value.g);
    Serial.print(F(" bs="));
    Serial.print(currentColor.value.b);
    Serial.print(F(" w="));
    Serial.print(currentColor.value.w);
    strip.setPixelColor(led,currentColor.value.r,currentColor.value.g,currentColor.value.b,0);
    strip.show();
    Serial.println();
}