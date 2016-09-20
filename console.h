#ifndef __CONSOLE__H_
#define __CONSOLE__H_
#include <stdint.h>
#include "font.h"

//rendering color for text, background is the inverse (allowed: 0,1)
#define CONSOLE_TEXTCOLOR 0

void console_init(void);
void console_clear(void);

static void console_render_str(uint8_t line, uint8_t color, uint8_t *str);
void console_puts(uint8_t *str);
void console_putc(uint8_t c);
void console_render(void);

//you can define the console font here. make sure to use FIXED WIDTH fonts!
//make sure to set width and height properly
//#define CONSOLE_FONT font_system5x7
#define CONSOLE_FONT font_tomthumb3x5
#define CONSOLE_FONT_WIDTH  3
#define CONSOLE_FONT_HEIGHT 5

//calculate how many chars can be printed on the console
#define CONSOLE_BUFFER_SIZE_X (LCD_WIDTH  / (CONSOLE_FONT_WIDTH+1))
#define CONSOLE_BUFFER_SIZE_Y ((LCD_HEIGHT / (CONSOLE_FONT_HEIGHT+1))+1)

#endif //__CONSOLE__H_

