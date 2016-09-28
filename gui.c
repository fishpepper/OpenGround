/*
    Copyright 2016 fishpepper <AT> gmail.com

    This program is free software: you can redistribute it and/ or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http:// www.gnu.org/licenses/>.

    author: fishpepper <AT> gmail.com
*/


#include "gui.h"
#include "debug.h"
#include "config.h"
#include "console.h"
#include "storage.h"
#include "led.h"
#include "io.h"
#include "wdt.h"
#include "adc.h"
#include "delay.h"
#include "touch.h"
#include "screen.h"


static uint32_t gui_shutdown_pressed;
static uint32_t gui_active = 0;
static uint32_t gui_page;
static uint32_t gui_config_tap_detected;

void gui_init(void) {
    debug("gui: init\n"); debug_flush();
    gui_page = 1;
    gui_shutdown_pressed = 0;
    gui_config_tap_detected = 0;
}

uint32_t gui_running(void) {
    return gui_active;
}

void gui_render(void) {
    screen_fill(0);

    if (gui_page == 0) {
        // render debug console
        console_render();
    } else if (gui_page == 1) {
        gui_render_statusbar();
    } else if (gui_page == 2) {
        gui_render_statusbar();
        gui_render_sliders();
    } else if (gui_page == 10) {
        gui_config_render_stick_calibration();
    } else {
        screen_fill(0);
        uint8_t buf[2];
        buf[0] = '0' + gui_page-2;
        buf[1] = 0;

        if (1) {  // gui_page == 2){
            screen_set_font(font_metric15x26);
            uint8_t buf2[10] = "00:00";
            buf2[1] = '0' + gui_page;
            screen_puts_xy(24, 1, 1, buf2);
        }
        // screen_puts_xy(64, 32, 1, buf);
    }
    screen_update();
}

static void gui_process_touch(void) {
    touch_event_t t = touch_get_and_clear_last_event();

    gui_config_tap_detected = 0;

    if (t.event_id == TOUCH_GESTURE_MOUSE_DOWN) {
        // there was a mouse click!

        // check for page next/prev clicks(on the right)
        if (t.x < GUI_PREV_CLICK_X) {
            // previous page:
            if (gui_page > 0) gui_page--;
        } else if (t.x > GUI_NEXT_CLICK_X) {
            // next page:
            if (gui_page < GUI_MAX_PAGE) {
                gui_page++;
            }
        }
        // check for tap during config
        gui_config_tap_detected = 1;
    }
}

void gui_handle_button_powerdown(void) {
    if (io_powerbutton_pressed()) {
        gui_shutdown_pressed++;
    } else {
        gui_shutdown_pressed = 0;
    }

    // shutdown animation for leds:
    if (gui_shutdown_pressed > GUI_SHUTDOWN_PRESS_COUNT_FROM_MS(200)) {
        // if pressed longer than 500ms, do shutdown blinking
        if ((gui_shutdown_pressed / GUI_SHUTDOWN_PRESS_COUNT_FROM_MS(200)) & 1) {
            led_button_l_on();
            led_button_r_off();
        } else {
            led_button_l_off();
            led_button_r_on();
        }
    }

    // debug_put_uint16(gui_shutdown_pressed);
    // debug_put_newline();
}

void gui_handle_buttons(void) {
    gui_handle_button_powerdown();
}

void gui_loop(void) {
    debug("gui: entering main loop\n"); debug_flush();
    gui_active = 1;
    gui_page = 10;

    // fixme: move this to a function
    uint32_t i;
    for (i = 0; i < 4; i++) {
        storage.stick_calibration[i][0] = adc_get_channel(i);
        storage.stick_calibration[i][1] = adc_get_channel(i)+1;
    }

    // this is the main GUI loop. rf stuff is done inside an ISR
    while (gui_shutdown_pressed < GUI_SHUTDOWN_PRESS_COUNT) {
        // process adc values
        adc_process();

        // handle buttons
        gui_handle_buttons();

        // handle touch gestures
        gui_process_touch();

        // render ui
        if (gui_page < 5) {
            // render normal ui
            gui_render();
        } else {
            // do config menu
            screen_fill(0);

            for (i = 0; i < 4; i++) {
                storage.stick_calibration[i][0] =
                        min(adc_get_channel(i), storage.stick_calibration[i][0]);
                storage.stick_calibration[i][1] =
                        max(adc_get_channel(i), storage.stick_calibration[i][1]);
            }

            gui_config_render_stick_calibration();
            if (gui_config_tap_detected) {
                // leave config
                gui_page = 1;
            }
            screen_update();
        }
        wdt_reset();
        delay_ms(GUI_LOOP_DELAY_MS);
    }

    debug("will power down now\n"); debug_flush();
    led_backlight_off();
    lcd_powerdown();
    io_powerdown();
}

static void gui_render_battery(void) {
    uint32_t x = 84;
    screen_set_font(GUI_STATUSBAR_FONT);

    screen_fill_rect(x-1, 0, LCD_WIDTH-x+1, 7, 1);

    // fetch battery voltage
    uint16_t v_bat = adc_get_battery_voltage();

    // show voltage
    screen_put_fixed2(x, 1, 0, v_bat);
    x += (GUI_STATUSBAR_FONT[FONT_FIXED_WIDTH]+1)*4;
    screen_puts_xy(x, 1, 0, "V");
    x += GUI_STATUSBAR_FONT[FONT_FIXED_WIDTH]+1;

    // render battery symbol
    x += 2;
    // draw border
    screen_draw_round_rect(x, 1, 21, 5, 2, 0);
    // show fillgrade
    // assume nimh batteries with 1.2V > 90% / 1.0V = 5%
    //                         = 4.8V       / 4.0V
    // i know this is not 100% correct, better calc is tbd ;)
    int32_t fill_percent = ((17 * v_bat)/ 16) - 420;
    // 0% = 0px, 100% = 20px
    int32_t fill_px = max(0, min(20, fill_percent / 5));
    // draw fill grade
    screen_fill_rect(x+1, 1+1, fill_px, 3, 0);
}

static void gui_render_rssi(uint8_t rssi_rx, uint8_t rssi_tx) {
    #define GUI_RSSI_BAR_W 25
    uint16_t x = 1;
    // render rx rssi bargraph at a given position
    screen_draw_round_rect(x, 1, GUI_RSSI_BAR_W+1, 5, 2, 1);
    x+=GUI_RSSI_BAR_W+2;
    // show values
    screen_puts_xy(x, 1, 1, "120|105");
    x += (GUI_STATUSBAR_FONT[FONT_FIXED_WIDTH]+1)*7;
    // render tx rssi bargraph at a given position
    screen_draw_round_rect(x, 1, GUI_RSSI_BAR_W+1, 5, 2, 1);
}



static void gui_render_statusbar(void) {
    // render rx/tx rssi and battery status:
    // draw divider
    screen_draw_line(0, 7, LCD_WIDTH, 7, 1);
    // draw battery voltage
    gui_render_battery();

    gui_render_rssi(111, 120);

    /*uint8_t buf[2];
    buf[0] = '0' + gui_page;
    buf[1] = 0;
    screen_puts_xy(64, 32, 1, buf);*/
}

static uint8_t gui_adc_channel_name(uint8_t i) {
    switch (i) {
        default: break;
        case(0): return 'A';
        case(1): return 'E';
        case(2): return 'T';
        case(3): return 'R';
        case(4):
        case(5):
        case(6):
        case(7): return '0'+(i-4);
    }
    return '?';
}

static void gui_render_sliders(void) {
    uint32_t i;
    uint32_t y;
    uint8_t str[5];

    screen_set_font(font_tomthumb3x5);

    for (i = 0; i < 8; i++) {
        uint32_t val = adc_get_channel(i);

        // render channel names
        str[0] = gui_adc_channel_name(i);
        str[1] = 0;
        y = 10 + i*(font_tomthumb3x5[FONT_HEIGHT]+1);
        screen_puts_xy(1, y, 1, str);

        // render sliders
        uint32_t y2 = y + (font_tomthumb3x5[FONT_HEIGHT]+1)/2;
        screen_draw_hline(8, y2 - 1, 50-1, 1);
        screen_draw_hline(8, y2 + 1, 50-1, 1);
        screen_draw_hline(8 + 50 + 1, y2 - 1, 50-1, 1);
        screen_draw_hline(8 + 50 + 1, y2 + 1, 50-1, 1);

        // render val as text
        screen_put_uint14(8 + 100 + 2, y, 1, val);

        // render value as slider 0..4096 to 0...100
        val = (val * 100) / 4096;
        screen_draw_vline(8 + val, y+1, 5, 1);
        screen_draw_vline(8 + val + 1, y+1, 5, 1);
    }
}

uint8_t *gui_channel_name(uint8_t i) {
    switch (i) {
        default  : return "???";
        case (0) : return "AIL";
        case (1) : return "ELE";
        case (2) : return "THR";
        case (3) : return "RUD";
        case (4) : return "CH0";
        case (5) : return "CH1";
        case (6) : return "CH2";
        case (7) : return "CH3";
    }
}

static void gui_config(void) {
    gui_config_render_stick_calibration();
}

static void gui_config_render_stick_calibration(void) {
    uint32_t idx;
    const uint8_t *font = font_tomthumb3x5;
    uint32_t h = font[FONT_HEIGHT] + 1;
    uint32_t w = font[FONT_FIXED_WIDTH] + 1;

    // do stick calibration
    screen_fill_rect(0, 0, LCD_WIDTH, 7, 1);
    screen_draw_round_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, 3, 1);

    screen_set_font(font);
    screen_puts_centered(1, 0, "STICK CALIBRATION");

    uint32_t y = 8;
    //                          |                          |
    screen_puts_xy(3, y, 1, "Please move all sticks to the"); y += h;
    screen_puts_xy(3, y, 1, "extreme positions."); y += h;
    screen_puts_xy(3, y, 1, "When done, move all sticks to"); y += h;
    screen_puts_xy(3, y, 1, "the center and tap the screen."); y += h;

    uint32_t x = 25;
    y = 33;
    screen_puts_xy(x+1*4*w+w, y, 1, "min");
    screen_puts_xy(x+2*4*w+1*2*w+w, y, 1, "now");
    screen_puts_xy(x+3*4*w+2*2*w+w, y, 1, "max");
    y += h;
    for (idx = 0; idx < 4; idx++) {
        screen_puts_xy(x, y, 1,                gui_channel_name(idx));
        screen_put_uint14(x+1*4*w, y, 1,       storage.stick_calibration[idx][0]);
        screen_put_uint14(x+2*4*w+1*2*w, y, 1, adc_get_channel(idx));
        screen_put_uint14(x+3*4*w+2*2*w, y, 1, storage.stick_calibration[idx][1]);
        y += h;
    }
}
