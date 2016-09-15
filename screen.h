#ifndef __SCREEN__H_
#define __SCREEN__H_
#include "config.h"
#include "lcd.h"
#include <stdint.h>

#define SCREEN_BUFFER_SIZE ((LCD_WIDTH * LCD_HEIGHT) / 8) 
static uint8_t screen_buffer[SCREEN_BUFFER_SIZE];

void screen_init(void);
void screen_clear(void);
void screen_update(void);


void screen_fill_round_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t radius, uint8_t color);
void screen_fill_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);
void screen_draw_round_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t radius, uint8_t color);
void screen_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);
void screen_draw_hline(uint8_t x, uint8_t y, uint8_t width, uint8_t color);
void screen_draw_round_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t radius, uint8_t color);
void screen_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);
void screen_draw_hline(uint8_t x, uint8_t y, uint8_t width, uint8_t color);
void screen_draw_vline(uint8_t x, uint8_t y, uint8_t height, uint8_t color);
void screen_set_pixels(uint8_t x, uint8_t y,uint8_t x2, uint8_t y2, uint8_t color);
void screen_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);

#define _screen_absDiff(x,y) (((x)>(y)) ?  ((x)-(y)) : ((y)-(x)))
#define _screen_swap(a,b) {uint8_t t; t=(a); a=(b);  b=t;}

#define screen_set_dot(x,y,color) { \
  if(((x) >= LCD_WIDTH) || ((y) >= LCD_HEIGHT)) { return; } \
  if (color){ \
    screen_buffer[((y)/8)*128 + (x)] |= (1<<((y)%8)); \
  }else{ \
    screen_buffer[((y)/8)*128 + (x)] &= ~(1<<((y)%8)); \
  } \
} 
        

#endif //__SCREEN__H_

