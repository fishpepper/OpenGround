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
#include "assert.h"

static uint32_t gui_config_counter;
static uint32_t gui_shutdown_pressed;
static uint32_t gui_active = 0;
static uint32_t gui_page;
static uint32_t gui_config_tap_detected;
static uint8_t gui_touch_callback_index;
static touch_callback_entry_t gui_touch_callback[GUI_TOUCH_CALLBACK_COUNT];

void gui_init(void) {
    debug("gui: init\n"); debug_flush();
    gui_page = 1;
    gui_shutdown_pressed = 0;
    gui_config_tap_detected = 0;
    gui_touch_callback_index = 0;

    gui_touch_callback_clear();
}

static void gui_touch_callback_clear(void) {
    uint32_t i;
    // debug("gui: touch callback clear\n");
    // debug_flush();

    // clear all touch callbacks
    for (i = 0; i < GUI_TOUCH_CALLBACK_COUNT; i++) {
        gui_touch_callback[i].callback = 0;
    }
    gui_touch_callback_index = 0;
}

static void gui_touch_callback_register(uint8_t xs, uint8_t xe, uint8_t ys, uint8_t ye,
                                        f_ptr_t cb) {
    // debug("gui: touch cb register\n");

    // check if we have space to register that slot:
    if (gui_touch_callback_index >= GUI_TOUCH_CALLBACK_COUNT) {
        // debug("gui: ERROR: no free slot\n");
        // debug_flush();
        return;
    }

    /* debug("     ");
    debug_put_uint8(xs); debug_putc('-'); debug_put_uint8(xe);
    debug_putc(' ');
    debug_put_uint8(ys); debug_putc('-'); debug_put_uint8(ye);
    debug_put_newline();
    debug_flush(); */

    // fine, configure this slot:
    gui_touch_callback[gui_touch_callback_index].xs       = xs;
    gui_touch_callback[gui_touch_callback_index].xe       = xe;
    gui_touch_callback[gui_touch_callback_index].ys       = ys;
    gui_touch_callback[gui_touch_callback_index].ye       = ye;
    gui_touch_callback[gui_touch_callback_index].callback = cb;
    gui_touch_callback_index++;
}

uint32_t gui_running(void) {
    return gui_active;
}


static void gui_touch_callback_execute(uint8_t i) {
    assert(i < gui_touch_callback_index);

    /*debug("gui: exec cb 0x");
    debug_put_hex32((uint32_t) gui_touch_callback[i].callback);
    debug_put_newline();
    debug_flush();*/

    if (gui_touch_callback[i].callback != 0) {
        // now execute this cabblack
        gui_touch_callback[i].callback();
    }
}

static void gui_add_button(uint8_t x, uint8_t y, const uint8_t *font, uint8_t *str, f_ptr_t cb) {
    uint32_t h = font[FONT_HEIGHT] + 1;
    uint32_t w = font[FONT_FIXED_WIDTH] + 1;

    // length of string
    uint8_t len = screen_strlen(str);

    // render a rounded rect
    uint32_t rect_w = len*w + 2*3;
    uint32_t rect_h = h + 2*2;
    screen_draw_round_rect(x, y, rect_w, rect_h, 3, 1);

    // add string
    screen_puts_xy(x + 3, y + 2, 1, str);

    // register the callback
    gui_touch_callback_register(x, x + rect_w, y, y + rect_h, cb);
}

static void gui_process_touch(void) {
    uint32_t i;

    // fetch pending touch event:
    touch_event_t t = touch_get_last_event();

    if (t.event_id == TOUCH_GESTURE_MOUSE_DOWN) {
        // there was a mouse click!
        // check if we will have to execute a callback
        for (i = 0; i < gui_touch_callback_index; i++) {
            // the first one matching will be triggered first.
            // anyway we also allow multiple triggers
            if (gui_touch_callback[i].callback != 0) {
                // check if click was inside this region
                if ((t.x >= gui_touch_callback[i].xs) && (t.x <= gui_touch_callback[i].xe) &&
                    (t.y >= gui_touch_callback[i].ys) && (t.y <= gui_touch_callback[i].ye) ) {
                        // execute callback!
                        gui_touch_callback_execute(i);
                }
            }
        }
    }
}

static void gui_cb_previous_page(void) {
    if (gui_page > 0) {
        gui_page--;
    }
}

static void gui_cb_next_page(void) {
    if (gui_page < GUI_MAX_PAGE) {
        gui_page++;
    }
}

static void gui_cb_config_back(void) {
    gui_page = GUI_PAGE_SETTING_FLAG | 0;
}

static void gui_cb_config_save(void) {
    storage_save();
    gui_cb_config_back();
}

static void gui_cb_config_stick_cal(void) {
    gui_page = GUI_PAGE_SETTING_FLAG | 1;
}

static void gui_cb_config_clonetx(void) {
    gui_config_counter = 0;
    gui_page = GUI_PAGE_SETTING_FLAG | 2;
}

static void gui_cb_config_model(void) {
    gui_page = GUI_PAGE_SETTING_FLAG | 3;
}

static void gui_cb_config_exit(void) {
    gui_page = 1;
}

void gui_handle_button_powerdown(void) {
    if (io_powerbutton_pressed()) {
        gui_shutdown_pressed++;
    } else {
        gui_shutdown_pressed = 0;
        led_button_r_off();
        led_button_l_off();
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
}

void gui_handle_buttons(void) {
    gui_handle_button_powerdown();
}

void gui_loop(void) {
    debug("gui: entering main loop\n"); debug_flush();
    gui_active = 1;
    gui_page = GUI_PAGE_SETTING_FLAG | 0;

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

        // clear old touch callbacks as the page rendering
        // will (re-)register callbacks
        gui_touch_callback_clear();

        // render ui
        if (gui_page & GUI_PAGE_SETTING_FLAG) {
            // render settings ui
            gui_config_render();
        } else {
            // render normal ui
            gui_render();
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

    // screen_fill_rect(x-1, 0, LCD_WIDTH-x+1, 7, 1);

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
    screen_fill_round_rect(x, 1, GUI_RSSI_BAR_W+1, 5, 2, 0);
    x+=GUI_RSSI_BAR_W+2;
    // show values
    screen_puts_xy(x, 1, 0, "120|105");
    x += (GUI_STATUSBAR_FONT[FONT_FIXED_WIDTH]+1)*7;
    // render tx rssi bargraph at a given position
    screen_fill_round_rect(x, 1, GUI_RSSI_BAR_W+1, 5, 2, 0);
}

static void gui_render_statusbar(void) {
    // render rx/tx rssi and battery status:
    // draw divider
    screen_fill_rect(0, 0, LCD_WIDTH, 7, 1);

    // draw battery voltage
    gui_render_battery();

    gui_render_rssi(111, 120);
}


static void gui_render_bottombar(void) {
    // render modelname at bottom
    // draw black border
    screen_fill_rect(0, LCD_HEIGHT - 7, LCD_WIDTH, 7, 1);

    screen_set_font(font_tomthumb3x5);
    screen_puts_centered(LCD_HEIGHT - 6, 0, storage.model[storage.current_model].name);
}


static uint8_t *gui_get_channel_name(uint8_t i, uint8_t type) {
    switch (i) {
        default  : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "?" : "???");
        case (0) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "A" : "AIL");
        case (1) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "E" : "ELE");
        case (2) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "T" : "THR");
        case (3) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "R" : "RUD");
        case (4) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "0" : "CH0");
        case (5) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "1" : "CH1");
        case (6) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "2" : "CH2");
        case (7) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "3" : "CH3");
    }
}

static void gui_render_sliders(void) {
    uint32_t i;
    uint32_t y;

    // add statusbar
    gui_render_statusbar();

    screen_set_font(font_tomthumb3x5);

    for (i = 0; i < 8; i++) {
        uint32_t val = adc_get_channel(i);

        // render channel names
        y = 10 + i*(font_tomthumb3x5[FONT_HEIGHT]+1);
        screen_puts_xy(1, y, 1, gui_get_channel_name(i, GUI_CHANNEL_DESCR_SHORT));

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


static void gui_config_stick_calibration_store_adc_values(void) {
    uint32_t i;
    for (i = 0; i < 4; i++) {
        // min
        storage.stick_calibration[i][0] =
                min(adc_get_channel(i), storage.stick_calibration[i][0]);
        // center
        storage.stick_calibration[i][1] = adc_get_channel(i);
        // max
        storage.stick_calibration[i][2] =
                max(adc_get_channel(i), storage.stick_calibration[i][2]);
    }
}



void gui_render(void) {
    // start with empty screen
    screen_fill(0);

    // register page inc/dec callbacks
    gui_touch_callback_register(0, GUI_PREV_CLICK_X, 0, LCD_HEIGHT,
                                &gui_cb_previous_page);
    gui_touch_callback_register(LCD_WIDTH-GUI_PREV_CLICK_X, LCD_WIDTH, 0, LCD_HEIGHT,
                                &gui_cb_next_page);

    switch (gui_page) {
        case(0) :
            // debug console
            console_render();
            break;

        case(1) :
            // status screen
            gui_render_statusbar();
            gui_render_bottombar();
            break;

        case(2) :
            // slider screen
            gui_render_sliders();
            break;

        default :
            screen_fill(0);
            uint8_t buf[2];
            buf[0] = '0' + gui_page-2;
            buf[1] = 0;

            if (1) {  // gui_page == 2){
                screen_set_font(font_metric15x26);
                    uint8_t buf2[10] = "00:00";
                buf2[1] = '0' + gui_page;
                screen_puts_xy(24, 1, 1, buf2);

                /*screen_set_font(font_metric7x12);
                static uint8_t start = 0;
                start++;
                if (start>100) start = 0;
                if ((start/2) % 3 == 0)
                    screen_puts_xy(start, 40, 1, "[");
                else if ((start/2) % 3 == 1)
                    screen_puts_xy(start, 40, 1, "<");
                else
                    screen_puts_xy(start, 40, 1, "=");*/
            }
            break;
    }
    screen_update();
}

static void gui_config_render(void) {
    // start with an empty page
    screen_fill(0);

    // register callbacks
    // none for now...

    switch (gui_page) {
        case (0 | GUI_PAGE_SETTING_FLAG) :
            // main settings menu
            gui_config_main_render();
            break;

        case (1 | GUI_PAGE_SETTING_FLAG) :
            // stick calibration
            gui_config_stick_calibration_render();
            break;

        case (2 | GUI_PAGE_SETTING_FLAG) :
            // clone tx
            gui_config_clonetx_render();
            break;

        case (3 | GUI_PAGE_SETTING_FLAG) :
            // model config
            gui_config_model_render();
            break;
    }

    screen_update();
}

static void gui_config_header_render(uint8_t *str) {
    // border + header string
    screen_fill_rect(0, 0, LCD_WIDTH, 7, 1);
    screen_draw_round_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, 3, 1);

    screen_set_font(font_tomthumb3x5);
    screen_puts_centered(1, 0, str);
}

static void gui_config_main_render(void) {
    uint32_t idx;
    const uint8_t *font = font_tomthumb3x5;
    uint32_t h = font[FONT_HEIGHT] + 1;
    uint32_t w = font[FONT_FIXED_WIDTH] + 1;
    screen_set_font(font);

    // header
    gui_config_header_render("MAIN CONFIGURATION");

    // render buttons and set callback
    gui_add_button(3, 12 + 0*18, font, "STICK CAL", &gui_cb_config_stick_cal);
    gui_add_button(3, 12 + 1*18, font, "CLONE  TX", &gui_cb_config_clonetx);
    gui_add_button(3, 12 + 2*18, font, "MODEL CFG", &gui_cb_config_model);

    // exit button
    gui_add_button(94, 34 + 1*18, font, " EXIT ", &gui_cb_config_exit);
}

static void gui_config_clonetx_render(void) {
    const uint8_t *font = font_tomthumb3x5;
    uint32_t h = font[FONT_HEIGHT]+1;
    uint32_t w = font[FONT_FIXED_WIDTH]+1;

    // header
    gui_config_header_render("CLONE TX");
    screen_puts_xy(3, 9, 1, "Put TX in Bind Mode now!");

    if (gui_config_counter >= 0) screen_puts_xy(3, 9 + 1*h, 1, "preparing bind...");
    if (gui_config_counter >= 1) screen_puts_xy(3, 9 + 2*h, 1, "preparing autotune");
    if (gui_config_counter >= 2) screen_puts_xy(3, 9 + 3*h, 1, "autotune running (takes long)");
    if (gui_config_counter >= 3){
        screen_puts_xy(3, 9 + 4*h, 1, "autotune done. freq offset 0x");
        screen_put_hex16(3+w*29, 9 + 4*h, 1, storage.frsky_freq_offset);
    }
    if (gui_config_counter >= 4) screen_puts_xy(3, 9 + 5*h, 1, "fetching hoptable (takes long)");
    if (gui_config_counter >= 6) {
        screen_puts_xy(3, 9 + 6*h, 1, "hoptable received. txid 0x");
        screen_put_hex16(3+w*26, 9 + 6*h, 1, storage.frsky_txid[0]);
        screen_put_hex16(3+w*28, 9 + 6*h, 1, storage.frsky_txid[1]);
    }
    if (gui_config_counter >= 7) screen_puts_xy(3, 9 + 7*h, 1, "done. please switch off now");

    switch (gui_config_counter) {
        default:
        case (0) :
            frsky_do_bind_prepare();
            gui_config_counter++;
            break;

        case (1) :
            frsky_autotune_prepare();
            gui_config_counter++;
            break;

        case (2) :
            // do autotune loop until done
            while (!frsky_autotune_do()) {
                /*screen_fill(0);
                console_render();
                screen_update();*/
                if (io_powerbutton_pressed()){
                    // abort!
                    gui_page = GUI_PAGE_SETTING_FLAG | 0;
                    return;
                }
            }
            gui_config_counter++;
            break;

        case (3) :
            frsky_autotune_finish();
            gui_config_counter++;
            break;

        case (4) :
            frsky_fetch_txid_and_hoptable_prepare();
            gui_config_counter++;
            break;

        case (5) :
            while (!frsky_fetch_txid_and_hoptable_do()) {
                /*screen_fill(0);
                console_render();
                screen_update();*/
                if (io_powerbutton_pressed()){
                    // abort!
                    gui_page = GUI_PAGE_SETTING_FLAG | 0;
                    return;
                }
            }
            gui_config_counter++;
            break;

        case (6) :
            screen_update();
            frsky_fetch_txid_and_hoptable_finish();
            gui_config_counter++;
            break;

        case (7) :
            frsky_do_bind_finish();
            gui_config_counter++;
            break;

        case (8) :
            // DONE. REBOOT NECESSARY
            break;
    }


    // gui_add_button(94, 34 + 1*18, font_tomthumb3x5, " BACK ", &gui_cb_config_back);
}

static void gui_config_model_render(void) {
    // header
    gui_config_header_render("MODEL SETTINGS");
    gui_add_button(94, 34 + 1*18, font_tomthumb3x5, " BACK ", &gui_cb_config_back);
}


static void gui_config_stick_calibration_render(void) {
    uint32_t idx;
    uint32_t a;

    const uint8_t *font = font_tomthumb3x5;
    uint32_t h = font[FONT_HEIGHT] + 1;
    uint32_t w = font[FONT_FIXED_WIDTH] + 1;

    // store adc values
    gui_config_stick_calibration_store_adc_values();

    // draw ui
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

    uint32_t x = 3;
    y = 33;
    screen_puts_xy(x+1*4*w+w, y, 1, "min");
    screen_puts_xy(x+2*4*w+1*2*w+w, y, 1, "now");
    screen_puts_xy(x+3*4*w+2*2*w+w, y, 1, "max");
    y += h;
    for (idx = 0; idx < 4; idx++) {
        screen_puts_xy(x, y, 1, gui_get_channel_name(idx, GUI_CHANNEL_DESCR_LONG));
        for (a = 0; a < 3; a++) {
            screen_put_uint14(x+(a+1)*4*w+a*2*w, y, 1,       storage.stick_calibration[idx][a]);
        }
        y += h;
    }

    // render buttons and set callback
    gui_add_button(94, 34 + 0*18, font, " SAVE ", &gui_cb_config_save);
    gui_add_button(94, 34 + 1*18, font, " BACK ", &gui_cb_config_back);
}
