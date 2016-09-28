/*
    Copyright 2016 fishpepper <AT> gmail.com

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

#ifndef GUI_H_
#define GUI_H_

#include <stdint.h>

#define GUI_PREV_CLICK_X 10
#define GUI_NEXT_CLICK_X (LCD_WIDTH - GUI_PREV_CLICK_X)
#define GUI_MAX_PAGE 3
#define GUI_STATUSBAR_FONT font_tomthumb3x5


#define GUI_LOOP_DELAY_MS 100
#define GUI_SHUTDOWN_PRESS_S 5.0
#define GUI_SHUTDOWN_PRESS_COUNT_FROM_MS(_ms) ((_ms)/GUI_LOOP_DELAY_MS)
#define GUI_SHUTDOWN_PRESS_COUNT (GUI_SHUTDOWN_PRESS_COUNT_FROM_MS(1000*GUI_SHUTDOWN_PRESS_S))


void gui_init(void);
void gui_loop(void);
uint32_t gui_running(void);

static void gui_process_touch(void);

static void gui_render_sliders(void);
static void gui_render_battery(void);
static void gui_render_statusbar(void);
static void gui_config_render_stick_calibration(void);

#endif  // GUI_H_
