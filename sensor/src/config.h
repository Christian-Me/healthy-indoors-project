/**************************************************************************
 * Neopixel
 **************************************************************************/

#define NEOPIXEL_PIN D4
#define LED_AMOUNT 1
#define LED_TYPE NEO_RGBW
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#define NEOPIXEL_MIN_BRIGHTNESS 50          // min brightness of color indicator
#define NEOPIXEL_MAX_BRIGHTNESS 150         // max brightness of color indicator
#define NEOPIXEL_INDICATOR_BRIGHTNESS 150   // brightness of sensor indicator
#define NEOPIXEL_REFRESH 2000               // refresh rate;
#define NEOPIXEL_MAX_SAMPLED_DEVICES 5      // maximum device to sample
#define NEOPIXEL_INDICATOR_FADE_STEP 1      // step to fade each loop 
#define NEOPIXEL_INDICATOR_FADE_WAIT 10     // step to fade each loop
#define NEOPIXEL_SINGLE_LED_VALUE 1         // for singe LED show 1=max, 2=average, 3=min