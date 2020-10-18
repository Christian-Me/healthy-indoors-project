#ifndef COLOR_H__
#define COLOR_H__

#include "utils.h"

struct color_ColorRGB {
   uint8_t     b;
   uint8_t     g;
   uint8_t     r;
};

struct color_ColorHSV {
   float       h; // 0 - 359 degrees
   uint8_t     s;
   uint8_t     v;
};

// easy way to convert packed color to rgbw values and back
union packedColor {           // 32 bit color
    uint32_t color;
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t w;
    } value;
};


/*!
   @brief    convert three 8 bit RGB levels to a 16 bit colour value. White component will be ignored!
    @param    color       32bit RGB(W) packed color
    @returns  16bit packed color
*/
uint16_t color32to16(packedColor color)
{
  return ((color.value.r & 0xF8) << 8) | ((color.value.g & 0xFC) << 3) | (color.value.b >> 3);
}
/*
 * Algorithm adapted from https://gist.github.com/hdznrrd/656996. Uses a little libmath.
 */
/*!
   @brief    Alternative convert HSV to RGB
    @param    hue         hue float value 16 bit 0-360
    @param    sat         saturation 0-255
    @param    value       brightness 0-255
    @returns  32bit packed color. white is alway 0
*/
// void colorHSV2RGB(struct color_ColorHSV const *hsv, struct color_ColorRGB *rgb) {
uint32_t colorHSV2RGB(float hue, uint8_t sat, uint8_t val) {
   int i;
   float f,p,q,t;
   float h, s, v;
   packedColor color;

   //expand the u8 hue in range 0->255 to 0->359* (there are problems at exactly 360)
   // h = 359.0 * ((float)hsv->h / 255.0);
   h = hue;
   if (h>=360) h=359.9;

   h = MAX(0.0, MIN(360.0, h));
   s = MAX(0.0, MIN(100.0, sat));
   v = MAX(0.0, MIN(100.0, val));

   s /= 100;
   v /= 100;
   if(s == 0) {
      // Achromatic (grey)
      color.value.r = color.value.g = color.value.b = round(v*255);
      return color.color;
   }

   h /= 60; // sector 0 to 5
   i = floor(h);
   f = h - i; // factorial part of h
   p = v * (1 - s);
   q = v * (1 - s * f);
   t = v * (1 - s * (1 - f));
   switch(i) {
      case 0:
         color.value.r = round(255*v);
         color.value.g = round(255*t);
         color.value.b = round(255*p);
         break;
      case 1:
         color.value.r = round(255*q);
         color.value.g = round(255*v);
         color.value.b = round(255*p);
         break;
      case 2:
         color.value.r = round(255*p);
         color.value.g = round(255*v);
         color.value.b = round(255*t);
         break;
      case 3:
         color.value.r = round(255*p);
         color.value.g = round(255*q);
         color.value.b = round(255*v);
         break;
      case 4:
         color.value.r = round(255*t);
         color.value.g = round(255*p);
         color.value.b = round(255*v);
         break;
      default: // case 5:
         color.value.r = round(255*v);
         color.value.g = round(255*p);
         color.value.b = round(255*q);
   }
   #ifdef SERIAL_TRACE
    Serial.print(F(" hsv2rgb r:"));
    Serial.print(color.value.r);
    Serial.print(F(" g:"));
    Serial.print(color.value.g);
    Serial.print(F(" b:"));
    Serial.println(color.value.b);
   #endif
   return color.color;
}

/*!
   @brief    Convert HSV to RGB
    @param    hue         hue value 16 bit 0-65535
    @param    sat         saturation 0-255
    @param    value       brightness 0-255
    @returns  32bit packed color. white is alway 0
*/
uint32_t colorHSV(uint16_t hue, uint8_t sat, uint8_t val) {

  uint8_t r, g, b;

  // Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
  // 0 is not the start of pure red, but the midpoint...a few values above
  // zero and a few below 65536 all yield pure red (similarly, 32768 is the
  // midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
  // each for red, green, blue) really only allows for 1530 distinct hues
  // (not 1536, more on that below), but the full unsigned 16-bit type was
  // chosen for hue so that one's code can easily handle a contiguous color
  // wheel by allowing hue to roll over in either direction.
  hue = (hue * 1530L + 32768) / 65536;
  // Because red is centered on the rollover point (the +32768 above,
  // essentially a fixed-point +0.5), the above actually yields 0 to 1530,
  // where 0 and 1530 would yield the same thing. Rather than apply a
  // costly modulo operator, 1530 is handled as a special case below.

  // So you'd think that the color "hexcone" (the thing that ramps from
  // pure red, to pure yellow, to pure green and so forth back to red,
  // yielding six slices), and with each color component having 256
  // possible values (0-255), might have 1536 possible items (6*256),
  // but in reality there's 1530. This is because the last element in
  // each 256-element slice is equal to the first element of the next
  // slice, and keeping those in there this would create small
  // discontinuities in the color wheel. So the last element of each
  // slice is dropped...we regard only elements 0-254, with item 255
  // being picked up as element 0 of the next slice. Like this:
  // Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
  // Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
  // Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
  // and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
  // the constants below are not the multiples of 256 you might expect.

  // Convert hue to R,G,B (nested ifs faster than divide+mod+switch):
  if(hue < 510) {         // Red to Green-1
    b = 0;
    if(hue < 255) {       //   Red to Yellow-1
      r = 255;
      g = hue;            //     g = 0 to 254
    } else {              //   Yellow to Green-1
      r = 510 - hue;      //     r = 255 to 1
      g = 255;
    }
  } else if(hue < 1020) { // Green to Blue-1
    r = 0;
    if(hue <  765) {      //   Green to Cyan-1
      g = 255;
      b = hue - 510;      //     b = 0 to 254
    } else {              //   Cyan to Blue-1
      g = 1020 - hue;     //     g = 255 to 1
      b = 255;
    }
  } else if(hue < 1530) { // Blue to Red-1
    g = 0;
    if(hue < 1275) {      //   Blue to Magenta-1
      r = hue - 1020;     //     r = 0 to 254
      b = 255;
    } else {              //   Magenta to Red-1
      r = 255;
      b = 1530 - hue;     //     b = 255 to 1
    }
  } else {                // Last 0.5 Red (quicker than % operator)
    r = 255;
    g = b = 0;
  }

  // Apply saturation and value to R,G,B, pack into 32-bit result:
  uint32_t v1 =   1 + val; // 1 to 256; allows >>8 instead of /255
  uint16_t s1 =   1 + sat; // 1 to 256; same reason
  uint8_t  s2 = 255 - sat; // 255 to 0
  return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
          (((((g * s1) >> 8) + s2) * v1) & 0xff00)       |
         ( ((((b * s1) >> 8) + s2) * v1)           >> 8);
}

/*!
   @brief       get luma value out of rgb values
   @details     rough calculation sufficient to devide to draw dark on bright background or white on dark backgrounds
    @param      packedColorValue         32bit color value
    @returns    luma value 0-255
*/
uint16_t getLuma(uint32_t packedColorValue) {
  packedColor color;
  color.color=packedColorValue;
  return (0.375 * color.value.r) + (0.5 * color.value.g) + (0.125 * color.value.b);
}

#endif