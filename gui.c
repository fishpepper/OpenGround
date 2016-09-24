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

#include "gui.h"
#include "debug.h"
#include "config.h"
#include "console.h"
#include "led.h"
#include "adc.h"
#include "delay.h"
#include "touch.h"
#include "screen.h"


static uint32_t gui_active = 0;
static uint32_t gui_page;

void gui_init(void){
    debug("gui: init\n"); debug_flush();
    gui_page = 1;
}

uint32_t gui_running(void){
    return gui_active;
}

void gui_render(void){
    if (gui_page == 0){
        //render debug console
        console_render();
    }else if(gui_page == 1){
        gui_render_statusbar();
    }else{
        screen_fill(0);
        uint8_t buf[2];
        buf[0] = '0' + gui_page;
        buf[1] = 0;
        screen_puts_xy(64, 32, 1, buf);
        screen_update();
    }
}

static void gui_process_touch(void){
    touch_event_t t = touch_get_and_clear_last_event();
    if (t.event_id == TOUCH_GESTURE_MOUSE_DOWN){
        //there was a mouse click!

        //check for page next/prev clicks (on the right)
        if (t.x < GUI_PREV_CLICK_X){
            //previous page:
            if (gui_page > 0) gui_page--;
        }else if (t.x > GUI_NEXT_CLICK_X){
            //next page:
            if (gui_page < GUI_MAX_PAGE){
                gui_page++;
            }
        }
    }
}

void gui_loop(void){
    debug("gui: entering main loop\n"); debug_flush();
    gui_active = 1;

    //for now use this 60s timeout to shutdown. do proper button handling later
    uint32_t delay = 60;
    uint32_t powerdown_counter = 10*(1000/delay);

    //this is the main GUI loop. rf stuff is done inside an ISR
    while(1){ //powerdown_counter--){
        //process adc values
        adc_process();

        //handle touch gestures
        gui_process_touch();

        //render ui
        gui_render();

        delay_ms(delay);
    }


    debug("will power down now\n"); debug_flush();
    lcd_powerdown();
    io_powerdown();
}

static void gui_render_battery(void){
    uint32_t x = 84;
    screen_set_font(GUI_STATUSBAR_FONT);

    screen_fill_rect(x-1,0,LCD_WIDTH-x+1,7, 1);

    //fetch battery voltage
    uint16_t v_bat = adc_get_battery_voltage();

    //show voltage
    screen_put_fixed2(x, 1, 0, v_bat);
    x += (GUI_STATUSBAR_FONT[FONT_FIXED_WIDTH]+1)*4;
    screen_puts_xy(x, 1, 0, "V");
    x += GUI_STATUSBAR_FONT[FONT_FIXED_WIDTH]+1;

    //render battery symbol
    x += 2;
    //draw border
    screen_draw_round_rect(x, 1, 21, 5, 2, 0);
    //show fillgrade
    //assume nimh batteries with 1.2V > 90% / 1.0V = 5%
    //                         = 4.8V       / 4.0V
    //i know this is not 100% correct, better calc is tbd ;)
    int32_t fill_percent = ((17 * v_bat)/16) - 420;
    //0% = 0px, 100% = 20px
    int32_t fill_px = max(0, min(20, fill_percent / 5));
    //draw fill grade
    screen_fill_rect(x+1, 1+1, fill_px, 3, 0);
}

static void gui_render_rssi(uint8_t rssi_rx, uint8_t rssi_tx){
    #define GUI_RSSI_BAR_W 25
    uint16_t x = 1;
    //render rx rssi bargraph at a given position
    screen_draw_round_rect(x, 1, GUI_RSSI_BAR_W+1, 5, 2, 1);
    x+=GUI_RSSI_BAR_W+2;
    //show values
    screen_puts_xy(x, 1, 1, "120|105" );
    x += (GUI_STATUSBAR_FONT[FONT_FIXED_WIDTH]+1)*7;
    //render tx rssi bargraph at a given position
    screen_draw_round_rect(x, 1, GUI_RSSI_BAR_W+1, 5, 2, 1);

}



static void gui_render_statusbar(void){
    //render rx/tx rssi and battery status:
    screen_fill(0);
    //draw divider
    screen_draw_line(0, 7, LCD_WIDTH, 7, 1);
    //draw battery voltage
    gui_render_battery();

    gui_render_rssi( 111, 120);

    uint8_t buf[2];
    buf[0] = '0' + gui_page;
    buf[1] = 0;
    screen_puts_xy(64, 32, 1, buf);
    screen_update();
}
