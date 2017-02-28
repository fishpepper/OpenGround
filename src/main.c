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

#include "main.h"
#include "clocksource.h"
//#include "timeout.h"
#include "config.h"
#include "delay.h"
#include "sound.h"
#include "led.h"
#include "io.h"
#include "lcd.h"
#include "debug.h"
#include "screen.h"
//#include "console.h"
//#include "adc.h"
//////#include "sound.h"
//#include "touch.h"
//#include "cc2500.h"
//#include "frsky.h"
//#include "storage.h"
//#include "wdt.h"
//#include "gui.h"
//#include "eeprom.h"

int main(void) {
    clocksource_init();
    config_init();
    delay_init();


    led_init();
    led_backlight_on();

#if 0
//    wdt_init();

#endif
    //io_init();
    timeout_init();

    lcd_init();

    lcd_show_logo();
    screen_init();


    console_init();
    debug_init();
    //adc_init();
    //sound_init();

    //touch_init();
    //eeprom_init();
    //storage_init();
    //frsky_init();

    //// screen_test();
    //// touch_test();
    //// adc_test();
    gui_init();

    debug("main: init done.\n"); debug_flush();
    // frsky_main();

    gui_loop();

    while (1) {
        // adc_test();
        // io_powerdown_test();
        // debug("ABCDEFGHIJKLMN\ni   .   !\n"); debug_flush();
        // while (1);
        //
    }


}

