#ifndef CHART_CALLBACKS_H__
#define CHART_CALLBACKS_H__

// draw color bar chart
void drawColorBar(uint8_t index, int sample,uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t valueBefore, uint8_t value, float scale = 1, uint16_t color = 0) {
    if (sample<0) return; // full erase not necessary
    if (sample==0) { // only erase area above first sample which is the biggest value
        display.drawFastVLine(x+index+1,y+1,h-value,MATRIX_BACKGROUND); // erase anything above
    }
    display.drawFastVLine(x+index+1,y+h-value-1,value-1,color);
}
// draw line chart
void drawColorLine(uint8_t index, int sample,uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t valueBefore, uint8_t value, float scale = 1, uint16_t color = 0) {
    if (sample>1) {
        display.drawLine(x+index+1,y+h-valueBefore-1,x+index+2,y+h-value-1,color);
    }
}

#endif