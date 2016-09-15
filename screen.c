/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

   author: fishpepper <AT> gmail.com
*/

//lcd drawing functions based on openglcd: https://bitbucket.org/bperrybap/openglcd/src

#include "screen.h"

static uint8_t screen_buffer[SCREEN_BUFFER_SIZE];

void screen_init(void) {
    screen_clear();
    //screen_draw_line(0,8,32,8,1);
    //screen_set_pixels(8,8,16,64,1);
    screen_fill_round_rect(128/2-60/2,64/2-30/2, 60, 30, 6, 1);
    screen_update();
}

void screen_clear(void) {
    uint32_t i;
    for(i=0; i<SCREEN_BUFFER_SIZE; i++){
        screen_buffer[i] = 0;
    }
    screen_update();
}

void screen_update(void) {
    lcd_send_data(screen_buffer, SCREEN_BUFFER_SIZE);
}

void screen_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color){
    uint8_t deltax, deltay, x,y, steep;
    int8_t error, ystep;

    steep = _screen_absDiff(y1,y2) > _screen_absDiff(x1,x2);  

    if (steep) {
        _screen_swap(x1, y1);
        _screen_swap(x2, y2);
    }

    if (x1 > x2){
        _screen_swap(x1, x2);
        _screen_swap(y1, y2);
    }

    deltax = x2 - x1;
    deltay =_screen_absDiff(y2,y1);  
    error = deltax / 2;
    y = y1;

    if(y1 < y2){
        ystep = 1;
    } else {
        ystep = -1;
    }

    for(x = x1; x <= x2; x++){
        if (steep){
            screen_set_dot(y,x, color); 
        }else{
            screen_set_dot(x,y, color);
        }
        error = error - deltay;

        if (error < 0){
            y = y + ystep;
            error = error + deltax;
        }
    }
}


// set pixels from upper left edge x,y to lower right edge x1,y1 to the given color
// the width of the region is x1-x + 1, height is y1-y+1 
void screen_set_pixels(uint8_t x, uint8_t y,uint8_t x2, uint8_t y2, uint8_t color){
    uint8_t mask, pageOffset, h, i, data;
    uint8_t height = y2-y+1;
    uint8_t width = x2-x+1;
    uint16_t dpos = 0;
        
    pageOffset = y%8;
    y -= pageOffset;
    mask = 0xFF;
    if(height < 8-pageOffset){
        mask >>= (8-height);
        h = height;
    }else{
        h = 8-pageOffset;
    }
    mask <<= pageOffset;

    dpos = (y/8)*128 + x;
    for(i=0; i < width; i++){
        if(color){
            screen_buffer[dpos] |= mask;
        } else {
            screen_buffer[dpos] &= ~mask;
        }
        dpos++;
    }

    while(h+8 <= height){
        h += 8;
        y += 8;
        dpos = (y/8)*128 + x;
        for(i=0; i <width; i++){
        if (color){
            screen_buffer[dpos] = 0xFF;
        }else{
            screen_buffer[dpos] = 0x00;
        }
        dpos++;
        }
    }
    
    if(h < height){
        mask = ~(0xFF << (height-h));
        dpos = (y/8+1)*128 + x;
        for(i=0; i < width; i++){
        if(color){
            screen_buffer[dpos] |= mask;
        }else{
            screen_buffer[dpos] &= ~mask;
        }
        dpos++;      
        }
    }
}

void screen_draw_vline(uint8_t x, uint8_t y, uint8_t height, uint8_t color){
    screen_set_pixels(x,y,x,y+height-1,color);
}

void screen_draw_hline(uint8_t x, uint8_t y, uint8_t width, uint8_t color){
    screen_set_pixels(x,y, x+width-1, y, color);
}

void screen_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color){
    //top
    screen_draw_hline(x, y, width, color);    
    // bottom    
    screen_draw_hline(x, y+height-1, width, color);   
    // left
    screen_draw_vline(x, y, height, color);   
    // right
    screen_draw_vline(x+width-1, y, height, color);   // right
}

void screen_draw_round_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t radius, uint8_t color){
    int16_t tSwitch; 
    uint8_t x1 = 0, y1 = radius;
    tSwitch = 3 - 2 * radius;

    while (x1 <= y1){
        // upper left corner
        screen_set_dot(x+radius - x1, y+radius - y1, color); // upper half
        screen_set_dot(x+radius - y1, y+radius - x1, color); // lower half

        // upper right corner
        screen_set_dot(x+width-radius-1 + x1, y+radius - y1, color); // upper half
        screen_set_dot(x+width-radius-1 + y1, y+radius - x1, color); // lower half

        // lower right corner
        screen_set_dot(x+width-radius-1 + x1, y+height-radius-1 + y1, color); // lower half
        screen_set_dot(x+width-radius-1 + y1, y+height-radius-1 + x1, color); // upper half

        // lower left corner
        screen_set_dot(x+radius - x1, y+height-radius-1 + y1, color); // lower half
        screen_set_dot(x+radius - y1, y+height-radius-1 + x1, color); // upper half

        if (tSwitch < 0){
            tSwitch += (4 * x1 + 6);
        }else{
            tSwitch += (4 * (x1 - y1) + 10);
            y1--;
        }
        x1++;
    }
        
    screen_draw_hline(x+radius, y, width-(2*radius), color);      // top
    screen_draw_hline(x+radius, y+height-1, width-(2*radius), color); // bottom
    screen_draw_vline(x, y+radius, height-(2*radius), color);     // left
    screen_draw_vline(x+width-1, y+radius, height-(2*radius), color); // right
}

void screen_fill_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color){
    screen_set_pixels(x,y,x+width-1,y+height-1,color);
}

void screen_fill_round_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t radius, uint8_t color){
    int16_t tSwitch; 
    uint8_t x1 = 0, y1 = radius;
    tSwitch = 3 - 2 * radius;
    
    // center block
    // filling center block first makes it apear to fill faster
    screen_fill_rect(x+radius, y, width-2*radius, height, color);

    while (x1 <= y1){
        // left side
        screen_draw_line(
            x+radius - x1, y+radius - y1,     // upper left corner upper half
            x+radius - x1, y+height-radius-1 + y1,  // lower left corner lower half
            color);
        screen_draw_line(
            x+radius - y1, y+radius - x1,     // upper left corner lower half
            x+radius - y1, y+height-radius-1 + x1,  // lower left corner upper half
            color);

        // right side
        screen_draw_line(
            x+width-radius-1 + x1, y+radius - y1,     // upper right corner upper half
            x+width-radius-1 + x1, y+height-radius-1 + y1, // lower right corner lower half
            color);
        screen_draw_line(
            x+width-radius-1 + y1, y+radius - x1,     // upper right corner lower half
            x+width-radius-1 + y1, y+height-radius-1 + x1,  // lower right corner upper half
            color);

        if (tSwitch < 0) {
            tSwitch += (4 * x1 + 6);
        } else {
            tSwitch += (4 * (x1 - y1) + 10);
            y1--;
        }
        x1++;
    }
}

