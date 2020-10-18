#ifndef MONITOR_CALLBACKS_H__
#define MONITOR_CALLBACKS_H__

/*********************************************************************************
 monitor_callbacks.h contains all functions for styling the user interface:

 - color callbacks: return a color for a given value

 - alarm callbacks: return a alarm code for a given value (oriented on IAQ classification)
   0= technical issues: reading unreliable
   1= excellent
   2= good
   3= acceptable: action necessary
   4= moderate: more actions necessary
   5= bad: immediate action necessary
   6= severe: search for cause, leave room
   7= extreme: leave room, search for cause 

*********************************************************************************/

// callback for the alarm level for IAQ values
uint8_t getIAQAlarm(float value) {
    if (value<50) return 1;
    if (value<100) return 2;
    if (value<150) return 3;
    if (value<200) return 4;
    if (value<250) return 5;
    if (value<350) return 6;
    return 7;
}
// callback for the alarm level for Humidity values
// figures roughly taken from https://jvi.asm.org/content/88/14/7692.long
// the study shows an effect by temperature & humidity in corelation on influenza virus!
// this is not jet implemented!
uint8_t getHumidityAlarm(float value) {
    if (value<35) return 7;
    if (value<40) return 6;
    if (value<42) return 3;
    if (value<45) return 2;
    if (value<55) return 1;
    if (value<68) return 2;
    if (value<70) return 3;
    if (value<72) return 4;
    if (value<75) return 5;
    if (value<80) return 6;
    return 7;
}
// callback for the alarm level for Temperature values
// figures roughly taken from https://jvi.asm.org/content/88/14/7692.long
// the study shows an effect by temperature & humidity in corelation on influenza virus!
// this is not jet implemented!
uint8_t getTemperatureAlarm(float value) {
    if (value<10) return 7;
    if (value<12) return 6;
    if (value<15) return 3;
    if (value<18) return 2;
    return 1;
}
// callback for the alarm level for CO2 values
// figures roughly taken from 
uint8_t getCO2Alarm(float value) {
    return 1;
}

// callback function the define color for IAQ values
uint32_t getIAQColor(float value) {
    float colorHue = 0;
    float saturation = 255;
    float brightness = 255;
    if (value<200) { // hue from 120 down to 30 = 90 degree [OK]
        colorHue = (float)90/200*(value);
        colorHue = 120-colorHue;
    } else if (value<350) { // hue form 30-0 and 360-300 = 90 degree [BAD]
        colorHue = (float)90/150*(value-200);
        colorHue = 30-colorHue;
        if (colorHue<0) colorHue=360+colorHue;
    } else { // value 350-500 hue from 300-280 = 20 degree & (sat 255-50) & brightness 255-100 [DANGER]
        colorHue = (float)20/150*(value-350);
        colorHue = 300-colorHue;
        //saturation = (float)205/150*(value-350);
        //saturation = 255-saturation; 
        brightness = (float)155/150*(value-350);
        brightness = 255-brightness; 
    }
    #ifdef SERIAL_TRACE
        Serial.print(F(" IAQ value="));
        Serial.print(value);
        Serial.print(F(" hue="));
        Serial.print(colorHue);
        Serial.print(F(" saturation="));
        Serial.print(saturation);
        Serial.print(F(" brightness="));
        Serial.println(brightness);
    #endif
    return colorHSV2RGB(colorHue,floor(saturation),floor(brightness));
}
void drawIAQGrid(uint16_t x,uint16_t y,uint16_t w,uint16_t h) {
    // display.fillRect(x, y, w, h, TFT_RED);
}
// callback function the define color for CO2 values
uint32_t getCO2Color(float value) {
    uint16_t colorHue = floor(value * (65535 / 5000)); // CO2 goes from 0-5000ppm
    return colorHSV(colorHue,0,20);
}
// callback function the define color for VOC values
uint32_t getVOCColor(float value) {
    uint16_t colorHue = floor(value * (65535 / 1000)); // VOC goes from 0-500
    return colorHSV(colorHue,0,20);
}
// callback function the define color for Temperature values
uint32_t getTemperatureColor(float value) {
    float colorHue = 0;
    float saturation = 255;
    float brightness = 255;
    if (value<15) { // hue form 0 to 60 = 60 degree [BAD]
        colorHue = (float)60/17*(value-15);
        if (colorHue<0) colorHue=0;
    } else if (value<18) { // hue from 60 to to 120 = 60 degree [OK]
        colorHue = (float)60/3*(value-18);
        colorHue = 60+colorHue;
    } else if (value<22) { // hue 120 [OK]
        colorHue = 120;
    } else if (value<30) { // hue from 120 to to 60 = 60 degree [OK]
        colorHue = (float)60/8*(value-22);
        colorHue = 120-colorHue;
    } else { // hue form 30-0 and 360-300 = 90 degree [BAD]
        colorHue = (float)50/10*(value-30);
        colorHue = 60-colorHue;
        if (colorHue<0) colorHue=0;
    }
    return colorHSV2RGB(colorHue,floor(saturation),floor(brightness));
}

uint32_t getHumidityColor(float value) {    
    float colorHue = 0;
    float saturation = 255;
    float brightness = 255;
    if (value<35) { // hue form 300 to 360 = 60 degree [BAD]
        colorHue = (float)60/35*(value-35);
        colorHue = 300+colorHue;
    } else if (value<42) { // hue form 0 to 60 = 60 degree [BAD]
        colorHue = (float)60/7*(value-35);
    } else if (value<50) { // hue from 60 to 120 = 60 degree [OK]
        colorHue = (float)60/8*(value-42);
        colorHue = 60+colorHue;
    } else if (value<58) { // hue from 120 to 60 = 60 degree [OK]
        colorHue = (float)60/8*(value-50);
        colorHue = 120-colorHue;
    } else if (value<70) { // hue from 60 to 0 = 60 degree [OK]
        colorHue = (float)60/12*(value-58);
        colorHue = 60-colorHue;
    } else if (value<75) { // hue from 0 to 60 = 60 degree [OK]
        colorHue = (float)60/5*(value-70);
    } else if (value<85) { // hue from 60 to 120 = 60 degree [OK]
        colorHue = (float)60/10*(value-75);
        colorHue = 60+colorHue;
    } else { // hue form 120-60 = 60 degree [BAD]
        colorHue = (float)60/15*(value-85);
        colorHue = 120-colorHue;
    }
    return colorHSV2RGB(colorHue,floor(saturation),floor(brightness));
}

#endif