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
#include "timeout.h"
#include "config.h"
#include "delay.h"
#include "sound.h"
#include "led.h"
#include "io.h"
#include "lcd.h"
#include "debug.h"
#include "screen.h"
#include "console.h"
#include "adc.h"
// #include "sound.h"
#include "touch.h"
#include "cc2500.h"
#include "frsky.h"
#include "storage.h"
#include "wdt.h"
#include "gui.h"
#include "eeprom.h"


// Define our function pointer
void (*bootloader_jump)(void);

static void handle_bootloader_request(void) {
    // Check if we should go into bootloader mode.
    //
    // Set the main stack pointer __set_MSP() to its default value.  The default
    // value of the main stack pointer is found by looking at the default value
    // in the System Memory start address. Do this in IAR View -> Memory.  I
    // tried this and it showed address: 0x200014A8 which I then tried here.
    // The IAR compiler complained that it was out of range.  After some
    // research, I found the following from "The STM32 Cortex-M0 Programming
    // Manual":
    //         Main Stack Pointer (MSP)(reset value). On reset, the processor
    //         loads the MSP with the value from address 0x00000000.
    //
    // So I then looked at the default value at address 0x0 and it was 0x20002250
    //
    // Note that 0x1fffC800 is "System Memory" start address for STM32 F0xx
    //
    if ( *((unsigned long *)0x20003FF0) == 0xDEADBEEF ) {
         *((unsigned long *)0x20003FF0) =  0xCAFEFEED; // Reset our trigger
        // 0x1fffC800 is "System Memory" start address for STM32 F0xx
        // point the PC to the System Memory reset vector (+4)
        bootloader_jump = (void (*)(void)) (*((uint32_t *) 0x1fffC804));
        bootloader_jump();
        while (1);
    }
}


int main(void) {
    // if this was a reboot with bootloader request enter
    // internal rom bootloader
    handle_bootloader_request();

    // init crystal osc & set clock options
    clocksource_init();

    config_init();

    delay_init();


    led_init();
    led_backlight_on();


//    wdt_init();

    io_init();
    timeout_init();

    lcd_init();

    lcd_show_logo();
    screen_init();


    console_init();
    debug_init();
    adc_init();
    sound_init();

    touch_init();
    eeprom_init();
    storage_init();

    frsky_init();

    /// screen_test();
    // touch_test();
    // adc_test();
    gui_init();

    debug("main: init done.\n"); debug_flush();
    //frsky_main();

    gui_loop();

    while (1) {
        // adc_test();
        // io_powerdown_test();
        // debug("ABCDEFGHIJKLMN\ni   .   !\n"); debug_flush();
        // while (1);
        //
    }
}

