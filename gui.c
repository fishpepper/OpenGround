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
#include "storage.h"
#include "wdt.h"
#include "adc.h"
#include "sound.h"
#include "delay.h"
#include "touch.h"
#include "screen.h"
#include "assert.h"

static uint32_t gui_config_counter;
static uint32_t gui_shutdown_pressed;
static uint8_t gui_active = 0;
static uint8_t gui_page;
static uint8_t gui_sub_page;
static uint8_t gui_config_tap_detected;
static uint8_t gui_touch_callback_index;
static touch_callback_entry_t gui_touch_callback[GUI_TOUCH_CALLBACK_COUNT];
static int16_t gui_model_timer;
static uint8_t gui_loop_counter;

void gui_init(void) {
    debug("gui: init\n"); debug_flush();
    gui_page = 0;
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

    // make sure to have valid ranges by ignoring bad ranges
    if (xs > xe) { xs = xe; }
    if (ys > ye) { ys = ye; }

    // fine, configure this slot:
    gui_touch_callback[gui_touch_callback_index].xs       = min(xs, LCD_WIDTH);
    gui_touch_callback[gui_touch_callback_index].xe       = min(xe, LCD_WIDTH);
    gui_touch_callback[gui_touch_callback_index].ys       = min(ys, LCD_HEIGHT);
    gui_touch_callback[gui_touch_callback_index].ye       = min(ye, LCD_HEIGHT);
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
        // execute this callback if valid
        gui_touch_callback[i].callback();
    }
}

static void gui_add_button_smallfont(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                                     uint8_t *str, f_ptr_t cb) {
    screen_set_font(font_tomthumb3x5);
    gui_add_button(x, y, w, h, str, cb);
}

static void gui_add_button(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t *str, f_ptr_t cb) {
    // render string
    screen_puts_xy_centered(x + w/2, y + h/2, 1, str);

    // render a rounded rect
    screen_draw_round_rect(x, y, w, h, 3, 1);

    // register the callback
    gui_touch_callback_register(x, x + w, y, y + h, cb);
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
                        // play sound
                        sound_play_click();
                        // execute callback!
                        gui_touch_callback_execute(i);
                }
            }
        }
    }
}

static void gui_cb_model_timer_reload(void) {
    gui_model_timer = (int16_t) storage.model[storage.current_model].timer;
}

static void gui_cb_model_prev(void) {
    if (storage.current_model > 0) {
        storage.current_model--;
    }
}

static void gui_cb_model_next(void) {
    if (storage.current_model < (STORAGE_MODEL_MAX_COUNT-1)) {
        storage.current_model++;
    }
}

static void gui_cb_setting_model_stickscale(void) {
    gui_page    |= GUI_PAGE_SETTING_OPTION_FLAG;
    gui_sub_page = GUI_SUBPAGE_SETTING_MODEL_SCALE;
}

static void gui_cb_setting_model_name(void) {
    gui_page    |= GUI_PAGE_SETTING_OPTION_FLAG;
    gui_sub_page = GUI_SUBPAGE_SETTING_MODEL_NAME;
}

static void gui_cb_setting_model_timer(void) {
    gui_page    |= GUI_PAGE_SETTING_OPTION_FLAG;
    gui_sub_page = GUI_SUBPAGE_SETTING_MODEL_TIMER;
}

static void gui_cb_setting_option_leave(void) {
    gui_page &= ~GUI_PAGE_SETTING_OPTION_FLAG;
}

static void gui_cb_model_stickscale_dec(void) {
    if (storage.model[storage.current_model].stick_scale > 2) {
        storage.model[storage.current_model].stick_scale--;
    }
}

static void gui_cb_model_stickscale_inc(void) {
    if (storage.model[storage.current_model].stick_scale < 100) {
        storage.model[storage.current_model].stick_scale++;
    }
}

static void gui_cb_model_timer_dec(void) {
    if (storage.model[storage.current_model].timer > 2) {
        storage.model[storage.current_model].timer--;
    }
}

static void gui_cb_model_timer_inc(void) {
    if (storage.model[storage.current_model].timer < 99*60) {
        storage.model[storage.current_model].timer++;
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
    uint32_t i, j;
    // channels to map
    uint8_t data_index[4];
    data_index[0] = ADC_CHANNEL_AILERON;
    data_index[1] = ADC_CHANNEL_ELEVATION;
    data_index[2] = ADC_CHANNEL_THROTTLE;
    data_index[3] = ADC_CHANNEL_RUDDER;

    // reinit min/center/max to current value
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3; j++) {
            storage.stick_calibration[i][j] = adc_get_channel(data_index[i]);
        }
    }

    gui_page = GUI_PAGE_SETTING_FLAG | 1;
}

static void gui_cb_config_clonetx(void) {
    // disable tx code
    frsky_tx_set_enabled(0);

    gui_config_counter = 0;
    gui_page = GUI_PAGE_SETTING_FLAG | 2;
}

static void gui_cb_config_model(void) {
    gui_page = GUI_PAGE_SETTING_FLAG | 3;
    gui_sub_page = 0;
}

static void gui_cb_config_enter(void) {
    gui_page = GUI_PAGE_SETTING_FLAG | 0;
}

static void gui_cb_config_exit(void) {
    // restore old settings
    storage_load();

    // restart tx code
    frsky_tx_set_enabled(1);

    // back to config main menu
    gui_page = 0;
}

void gui_handle_button_powerdown(void) {
    if (io_powerbutton_pressed()) {
        gui_shutdown_pressed++;
    } else {
        if (gui_shutdown_pressed) {
            // reset counter and switch off leds
            gui_shutdown_pressed = 0;
            led_button_r_off();
            led_button_l_off();
        }
    }

    // shutdown animation for leds:
    if (gui_shutdown_pressed > GUI_SHUTDOWN_PRESS_COUNT_FROM_MS(200)) {
        // if pressed longer than 200ms, do shutdown blinking
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

static void gui_process_logic(void) {
    uint32_t second_elapsed = 0;

    if (timeout2_timed_out()) {
        // one second has passed
        second_elapsed = 1;
        // next timeout in 1s
        timeout2_set_100us(10000);
    }

    // count down when
    if (adc_get_channel_rescaled(ADC_CHANNEL_THROTTLE) >= ADC_RESCALED_ZERO_THRESHOLD) {
        // do timer logic, handle countdown
        if (second_elapsed) {
            gui_model_timer--;
        }
    }
}



void gui_loop(void) {
    debug("gui: entering main loop\n"); debug_flush();
    gui_active = 1;

    // start with main page
    gui_page = 0;
    gui_loop_counter = 0;

    // re init model timer
    gui_cb_model_timer_reload();

    gui_page = 0;  // GUI_PAGE_SETTING_FLAG | 3;

    // prepare timeout
    timeout_set_100us(10 * GUI_LOOP_DELAY_MS);

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

        // do some ui logic, like counting down timers,
        // doing warning beeps etc
        gui_process_logic();

        // render ui
        if (adc_get_channel_rescaled(ADC_CHANNEL_CH3) < 0) {
            // show console on switch down
            console_render();
            screen_update();
        } else if (gui_page & GUI_PAGE_SETTING_FLAG) {
            // render settings ui
            gui_config_render();
        } else {
            // render normal ui
            gui_render();
        }

        gui_loop_counter++;

        wdt_reset();

        // wait for next gui iteration
        while (!timeout_timed_out()) {
            // do some processing instead of wasting cpu cycles
            frsky_handle_telemetry();
        }

        // prepare next timeout:
        timeout_set_100us(10 * GUI_LOOP_DELAY_MS);
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
    screen_draw_vline(x+20, 1, 5, 0);

    // show fillgrade
    // assume nimh batteries with 1.2V > 90% / 1.0V = 5%
    //                         = 4.8V       / 4.0V
    // i know this is not 100% correct, better calc is tbd ;)
    int32_t fill_percent = ((17 * v_bat)/ 16) - 420;
    fill_percent = max(min(fill_percent, 100), 5);

    // 0% = 0px, 100% = 20px
    int32_t fill_px = max(0, min(20, fill_percent / 5));
    // draw fill grade
    screen_fill_rect(x+1, 1+1, fill_px, 3, 0);
}

static void gui_render_rssi(uint8_t rssi_rx, uint8_t rssi_tx) {
    #define GUI_RSSI_BAR_W 25
    uint16_t x = 1;
    // render rx rssi bargraph at a given position
    screen_fill_rect(x, 1, GUI_RSSI_BAR_W+1, 5, 0);
    x+=GUI_RSSI_BAR_W+2;
    // show values
    screen_puts_xy(x, 1, 0, "120|105");

    // show RSSI
    uint8_t rssi, rssi_telemetry;
    frsky_get_rssi(&rssi, &rssi_telemetry);

    screen_put_uint8(x, 1, 0, rssi_telemetry);
    x += (GUI_STATUSBAR_FONT[FONT_FIXED_WIDTH]+1) * 3;
    screen_puts_xy(x, 1, 0, "|");
    x += (GUI_STATUSBAR_FONT[FONT_FIXED_WIDTH]+1) * 1;
    screen_put_uint8(x, 1, 0, rssi);
    x += (GUI_STATUSBAR_FONT[FONT_FIXED_WIDTH]+1) * 3;

    // render tx rssi bargraph at a given position
    screen_fill_rect(x, 1, GUI_RSSI_BAR_W+1, 5, 0);

    // fill bargraphs
    // rssi can be 0..100 (?)
    uint8_t bar_w = min(rssi_telemetry, 100)/4;
    if (bar_w > 1) bar_w--;
    if (bar_w > 0) screen_fill_rect(1+bar_w, 2, 25-bar_w, 3, 1);
    bar_w = min(rssi, 100)/4;
    if (bar_w > 1) bar_w--;
    if (bar_w > 0) screen_fill_rect(x+bar_w, 2, 25-bar_w, 3, 1);
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
    screen_puts_centered(LCD_HEIGHT - 6 + font_tomthumb3x5[FONT_HEIGHT]/2, 0,
                         storage.model[storage.current_model].name);
}


static uint8_t *gui_get_channel_name(uint8_t i, uint8_t type) {
    switch (i) {
        default  : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "?" : "???");
        case (ADC_CHANNEL_AILERON)   : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "A" : "AIL");
        case (ADC_CHANNEL_ELEVATION) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "E" : "ELE");
        case (ADC_CHANNEL_THROTTLE)  : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "T" : "THR");
        case (ADC_CHANNEL_RUDDER)    : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "R" : "RUD");
        case (ADC_CHANNEL_CH0) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "0" : "CH0");
        case (ADC_CHANNEL_CH1) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "1" : "CH1");
        case (ADC_CHANNEL_CH2) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "2" : "CH2");
        case (ADC_CHANNEL_CH3) : return ((type == GUI_CHANNEL_DESCR_SHORT) ? "3" : "CH3");
    }
}

static void gui_render_sliders(void) {
    uint32_t i;
    uint32_t y;

    // add statusbar
    gui_render_statusbar();

    screen_set_font(font_tomthumb3x5);

    uint8_t data_index[8];
    data_index[0] = ADC_CHANNEL_AILERON;
    data_index[1] = ADC_CHANNEL_ELEVATION;
    data_index[2] = ADC_CHANNEL_THROTTLE;
    data_index[3] = ADC_CHANNEL_RUDDER;
    data_index[4] = ADC_CHANNEL_CH0;
    data_index[5] = ADC_CHANNEL_CH1;
    data_index[6] = ADC_CHANNEL_CH2;
    data_index[7] = ADC_CHANNEL_CH3;

    for (i = 0; i < 8; i++) {
        uint8_t ch = data_index[i];

        // render channel names
        y = 10 + i*(font_tomthumb3x5[FONT_HEIGHT]+1);
        screen_puts_xy(1, y, 1, gui_get_channel_name(ch, GUI_CHANNEL_DESCR_SHORT));

        // render sliders
        uint32_t y2 = y + (font_tomthumb3x5[FONT_HEIGHT]+1)/2;
        screen_draw_hline(8, y2 - 1, 50-1, 1);
        screen_draw_hline(8, y2 + 1, 50-1, 1);
        screen_draw_hline(8 + 50 + 1, y2 - 1, 50-1, 1);
        screen_draw_hline(8 + 50 + 1, y2 + 1, 50-1, 1);

        int32_t val = adc_get_channel_rescaled(ch);
        // rescale  adc value from +/- 3200 to +/-100
        val = val / 32;

        // render val as text
        screen_put_int8(8 + 100 + 2, y, 1, val);

        // rescale from +/-100 to 0..100
        val = 50 + val/2;
        screen_draw_vline(8 + val - 1, y+1, 5, 1);
        screen_draw_vline(8 + val    , y+1, 5, 1);
    }
}


static void gui_config_stick_calibration_store_adc_values(void) {
    uint32_t i;
    uint8_t data_index[4];
    data_index[0] = ADC_CHANNEL_AILERON;
    data_index[1] = ADC_CHANNEL_ELEVATION;
    data_index[2] = ADC_CHANNEL_THROTTLE;
    data_index[3] = ADC_CHANNEL_RUDDER;

    for (i = 0; i < 4; i++) {
        uint8_t ch = data_index[i];

        // min
        storage.stick_calibration[ch][0] =
                min(adc_get_channel(ch), storage.stick_calibration[ch][0]);
        // center
        storage.stick_calibration[ch][1] = adc_get_channel(ch);
        // max
        storage.stick_calibration[ch][2] =
                max(adc_get_channel(ch), storage.stick_calibration[ch][2]);
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
        case (0) :
            // main status screen
            gui_render_main_screen();
            break;

        case (1) :
            // slider screen
            gui_render_sliders();
            break;

        default :
            gui_add_button_smallfont(64-50/2, 40, 50, 15, "SETUP", &gui_cb_config_enter);

            break;
    }
    screen_update();
}

static void gui_config_render(void) {
    // start with an empty page
    screen_fill(0);

    // register callbacks
    // none for now...
    switch (gui_page & GUI_PAGE_NOFLAGS) {
        case (0) :
            // main settings menu
            gui_config_main_render();
            break;

        case (1) :
            // stick calibration
            gui_config_stick_calibration_render();
            break;

        case (2) :
            // clone tx
            gui_config_clonetx_render();
            break;

        case (3) :
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
    screen_puts_centered(1 + font_tomthumb3x5[FONT_HEIGHT]/2, 0, str);
}

static void gui_render_main_screen(void) {
    uint32_t x;
    uint32_t y;

    // do statusbars
    gui_render_statusbar();
    gui_render_bottombar();

    // render voltage
    screen_set_font(font_metric7x12);
    x = 1;
    y = 10;
    screen_put_fixed2_1digit(x, y, 1, telemetry_get_voltage());
    x += (font_metric7x12[FONT_FIXED_WIDTH]+1)*3 + 3;
    screen_puts_xy(x, y, 1, "V");

    x = 1;
    y += font_metric7x12[FONT_HEIGHT]+1;
    screen_put_fixed2_1digit(x, y, 1, telemetry_get_current());
    x += (font_metric7x12[FONT_FIXED_WIDTH]+1)*3 + 3;
    screen_puts_xy(x, y, 1, "A");

    x = LCD_WIDTH - (font_metric7x12[FONT_FIXED_WIDTH]+1)*7 - 1;
    y += font_metric7x12[FONT_HEIGHT]+1;
    y += 5;
    screen_put_uint14(x, y, 1, telemetry_get_mah());
    x += (font_metric7x12[FONT_FIXED_WIDTH]+1)*4 + 1;
    screen_puts_xy(x, y, 1, "MAH");

    // screen_set_font(font_metric7x12);
    screen_set_font(font_metric15x26);

    // render time
    uint32_t color = 1;
    if (gui_model_timer < 0) {
        if ((gui_loop_counter % 4) == 0) {
            color = 1 - color;
        }
    }
    if ((gui_model_timer > 0) && (gui_model_timer < 15)) {
        if ((gui_loop_counter % 10) == 0) {
            // beep!
            sound_play_low_time();
        }
    }

    // render background
    x = 51;
    y = 10;
    uint32_t w = (font_metric15x26[FONT_FIXED_WIDTH]/2 + 2) +
                 (font_metric15x26[FONT_FIXED_WIDTH] + 1) * 4 + 3;
    uint32_t h = font_metric15x26[FONT_HEIGHT] + 2;
    screen_fill_round_rect(x, y, w, h, 2, 1 - color);
    x++;
    y++;

    // render time
    screen_put_time(x, y, color, gui_model_timer);
    // register the reset callback
    gui_touch_callback_register(x, x + w, y, y + h, &gui_cb_model_timer_reload);
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
    gui_add_button_smallfont(3, 10 + 0*17, 50, 15, "STICK CAL", &gui_cb_config_stick_cal);
    gui_add_button_smallfont(3, 10 + 1*17, 50, 15, "CLONE  TX", &gui_cb_config_clonetx);
    gui_add_button_smallfont(3, 10 + 2*17, 50, 15, "MODEL CFG", &gui_cb_config_model);

    // exit button
    gui_add_button_smallfont(74, 10 + 2*17, 50, 15, "EXIT", &gui_cb_config_exit);
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
    if (gui_config_counter >= 3) {
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
                // screen_fill(0); console_render(); screen_update();
                if (io_powerbutton_pressed()) {
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
                // screen_fill(0); console_render(); screen_update();
                if (io_powerbutton_pressed()) {
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


    // gui_add_button_smallfont(94, 34 + 1*18, font_tomthumb3x5, " BACK ", &gui_cb_config_back);
}

static void gui_config_model_render_main(void) {
    const uint8_t *font = font_system5x7;
    screen_set_font(font);

    uint32_t y = 12;

    // add model name
    screen_puts_centered(y, 1, storage.model[storage.current_model].name);
    // register the callback
    gui_touch_callback_register(20, LCD_WIDTH - 20, y, y + font[FONT_HEIGHT] + 1,
                                &gui_cb_setting_model_name);

    // add < ... > buttons
    gui_add_button_smallfont(3, y - 3, 15, 10, "<", &gui_cb_model_prev);
    gui_add_button_smallfont(LCD_WIDTH - 3 - 15, y - 3, 15, 10, ">", &gui_cb_model_next);
    y += 1 + font[FONT_HEIGHT];

    // add line
    // screen_draw_hline(0, y, LCD_WIDTH, 1);
    y += 2;

    // now use smaller font
    // font = font_tomthumb3x5;
    // screen_set_font(font);

    // add stick scaling
    gui_add_button_smallfont(3, y, 40, 13, "SCALE", &gui_cb_setting_model_stickscale);
    y += 13 + 1;

    // time
    gui_add_button_smallfont(3, y, 40, 13, "TIMER", &gui_cb_setting_model_timer);

    // render buttons and set callback
    gui_add_button_smallfont(89, 34 + 0*15, 35, 13, "SAVE", &gui_cb_config_save);
    gui_add_button_smallfont(89, 34 + 1*15, 35, 13, "BACK", &gui_cb_config_exit);
}


static void gui_render_option_window(uint8_t *opt_name, f_ptr_32_32_t func) {
    // render window
    // clear region for window
    uint32_t window_w = LCD_WIDTH - 20;
    uint32_t window_h = 55;
    uint32_t y = (LCD_HEIGHT - window_h) / 2;
    uint32_t x = (LCD_WIDTH - window_w) / 2;
    // clear
    screen_fill_round_rect(x, y , window_w, window_h, 4, 0);
    // render border
    screen_draw_round_rect(x, y, window_w, window_h, 4, 1);
    y += 5;

    // font selection
    const uint8_t *font = font_system5x7;
    screen_set_font(font);

    // render text
    screen_puts_centered(y, 1, opt_name);
    y += font[FONT_HEIGHT] + 1;
    uint32_t len = screen_strlen(opt_name);
    screen_draw_hline((LCD_WIDTH - len) / 2, y, len, 1);
    y += 5;

    // render change modifier
    if (func != 0) {
        func(x, y);
    }

    // add buttons
    screen_set_font(font_tomthumb3x5);
    y = LCD_HEIGHT - (LCD_HEIGHT - window_h) / 2 - 16;
    gui_add_button_smallfont((LCD_WIDTH - 40) / 2, y, 40, 13, "OK", &gui_cb_setting_option_leave);
}

static void gui_cb_render_option_stickscale(uint32_t x, uint32_t y) {
    screen_set_font(font_system5x7);

    // render +/- button
    gui_add_button(15, y, 15, 15, "-", &gui_cb_model_stickscale_dec);
    gui_add_button(LCD_WIDTH - 15 - 15, y, 15, 15, "+", &gui_cb_model_stickscale_inc);

    // render value
    screen_put_uint8(LCD_WIDTH / 2 - screen_strlen("123") / 2,
                     y, 1, storage.model[storage.current_model].stick_scale);
}

static void gui_cb_render_option_timer(uint32_t x, uint32_t y) {
    screen_set_font(font_system5x7);

    // render +/- button
    gui_add_button(15, y, 15, 15, "-", &gui_cb_model_timer_dec);
    gui_add_button(LCD_WIDTH - 15 - 15, y, 15, 15, "+", &gui_cb_model_timer_inc);

    // render timer value
    screen_put_time(LCD_WIDTH / 2 - screen_strlen("1234") / 2,
                     y, 1, storage.model[storage.current_model].timer);
}

static uint32_t gui_config_temp;

static void gui_config_model_render(void) {
    // header
    gui_config_header_render("MODEL SETTINGS");

    // render normal options
    gui_config_model_render_main();

    // single item selected?
    if (gui_page & GUI_PAGE_SETTING_OPTION_FLAG) {
        // render single settings above main!
        // but first: remove all button callbacks
        gui_touch_callback_clear();

        // which options have to be changed?
        switch (gui_sub_page) {
            default:
            case (GUI_SUBPAGE_SETTING_MODEL_NAME) :
                gui_render_option_window("MODEL NAME", 0);
                break;

            case (GUI_SUBPAGE_SETTING_MODEL_SCALE) :
                gui_render_option_window("STICK SCALE", &gui_cb_render_option_stickscale);
                break;

            case (GUI_SUBPAGE_SETTING_MODEL_TIMER) :
                gui_render_option_window("TIMER", &gui_cb_render_option_timer);
                break;
        }
    }
}


static void gui_config_stick_calibration_render(void) {
    uint32_t idx;
    uint8_t i;
    uint32_t a;

    const uint8_t *font = font_tomthumb3x5;
    uint32_t h = font[FONT_HEIGHT] + 1;
    uint32_t w = font[FONT_FIXED_WIDTH] + 1;

    // store adc values
    gui_config_stick_calibration_store_adc_values();

    // draw ui
    gui_config_header_render("STICK CALIBRATION");

    uint32_t y = 8;
    //                       |                             |
    screen_puts_xy(3, y, 1, "Please move all sticks to the"); y += h;
    screen_puts_xy(3, y, 1, "extreme positions."); y += h;
    screen_puts_xy(3, y, 1, "When done, move all sticks to"); y += h;
    screen_puts_xy(3, y, 1, "the center and press save."); y += h;

    uint32_t x = 3;
    y = 33;

    screen_puts_xy(x+1*4*w+w, y, 1,       "min");
    screen_puts_xy(x+2*4*w+1*2*w+w, y, 1, "now");
    screen_puts_xy(x+3*4*w+2*2*w+w, y, 1, "max");
    y += h;

    uint8_t data_index[4];
    data_index[0] = ADC_CHANNEL_AILERON;
    data_index[1] = ADC_CHANNEL_ELEVATION;
    data_index[2] = ADC_CHANNEL_THROTTLE;
    data_index[3] = ADC_CHANNEL_RUDDER;

    for (i = 0; i < 4; i++) {
        uint8_t idx = data_index[i];
        screen_puts_xy(x, y, 1, gui_get_channel_name(idx, GUI_CHANNEL_DESCR_LONG));
        for (a = 0; a < 3; a++) {
            screen_put_uint14(x+(a+1)*4*w+a*2*w, y, 1, storage.stick_calibration[idx][a]);
        }
        y += h;
    }

    // render buttons and set callback
    gui_add_button_smallfont(89, 34 + 0*15, 35, 13, "SAVE", &gui_cb_config_save);
    gui_add_button_smallfont(89, 34 + 1*15, 35, 13, "BACK", &gui_cb_config_exit);

    // gui_add_button_smallfont(94, 34 + 0*18, font, " SAVE ", &gui_cb_config_save);
    // gui_add_button_smallfont(94, 34 + 1*18, font, " BACK ", &gui_cb_config_back);
}
