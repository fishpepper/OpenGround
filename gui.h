#ifndef __GUI_H__
#define __GUI_H__
#include <stdint.h>

#define GUI_PREV_CLICK_X 10
#define GUI_NEXT_CLICK_X (LCD_WIDTH - GUI_PREV_CLICK_X)
#define GUI_MAX_PAGE 3
#define GUI_STATUSBAR_FONT font_tomthumb3x5

void gui_init(void);
void gui_loop(void);
uint32_t gui_running(void);


static void gui_render_battery(void);
static void gui_render_statusbar(void);
static void gui_process_touch(void);

#endif // __GUI_H__
