#ifndef __CONSOLE__H_
#define __CONSOLE__H_
#include <stdint.h>


//rendering color for text, background is the inverse (allowed: 0,1)
#define CONSOLE_TEXTCOLOR 1

void console_init(void);

static void console_render_str(uint8_t line, uint8_t color, uint8_t *str);
void console_puts(uint8_t *str);
void console_putc(uint8_t c);
void console_render(void);
//height of font is 7+1
//-> 64/8 = 8 lines
//width of font is 5+1
//->128/6 = 31 chars
#define CONSOLE_BUFFER_SIZE_X 21
#define CONSOLE_BUFFER_SIZE_Y 8


#endif //__CONSOLE__H_

