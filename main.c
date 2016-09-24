#include "main.h"
    #include "timeout.h"
#include "stm32f0xx_gpio.h"
#include "config.h"
#include "delay.h"
#include "led.h"
#include "io.h"
#include "lcd.h"
#include "debug.h"
#include "screen.h"
#include "console.h"
#include "adc.h"
#include "sound.h"
#include "touch.h"
#include "cc2500.h"
#include "frsky.h"
#include "storage.h"
#include "wdt.h"
#include "gui.h"

int main(void) {
    wdt_init();
    delay_init();
    led_init();
    io_init();

    timeout_init();
    lcd_init();

    screen_init();
    console_init();
    debug_init();
    adc_init();
    sound_init();
    touch_init();
    storage_init();
    frsky_init();

    //screen_test();
    //touch_test();
    gui_init();

    debug("main: init done.\n"); debug_flush();
    //frsky_main();

    gui_loop();

    while(1){
        //adc_test();
        //io_powerdown_test();
        //debug("ABCDEFGHIJKLMN\ni   .   !\n"); debug_flush();
        //while(1);
        //
    }

}

