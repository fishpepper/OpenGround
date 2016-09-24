#ifndef __GUI_H__
#define __GUI_H__
#include <stdint.h>

#define GUI_PREV_CLICK_X 10
#define GUI_NEXT_CLICK_X (LCD_WIDTH - GUI_PREV_CLICK_X)
#define GUI_MAX_PAGE 3
#define GUI_STATUSBAR_FONT font_tomthumb3x5


#define GUI_LOOP_DELAY_MS 50
#define GUI_SHUTDOWN_PRESS_S 5.0
#define GUI_SHUTDOWN_PRESS_COUNT_FROM_MS(_ms) ((_ms)/GUI_LOOP_DELAY_MS)
#define GUI_SHUTDOWN_PRESS_COUNT (GUI_SHUTDOWN_PRESS_COUNT_FROM_MS(1000*GUI_SHUTDOWN_PRESS_S))


void gui_init(void);
void gui_loop(void);
uint32_t gui_running(void);


static void gui_render_battery(void);
static void gui_render_statusbar(void);
static void gui_process_touch(void);

#endif // __GUI_H__
