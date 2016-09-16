#include "main.h"
#include "stm32f0xx_gpio.h"
#include "config.h"
#include "delay.h"
#include "led.h"
#include "io.h"
#include "lcd.h"
#include "screen.h"
#include "console.h"

int main(void) {
    delay_init();
    led_init();
    io_init();
   
    led_on(); 
    lcd_init();
    screen_init();
    console_init();
    debug_init();
    

    led_on();
    uint8_t countdown = 5;
    while(1){
        debug("power down in ");
        debug_put_uint8(countdown--);
        debug("s\n");

        console_render();
        delay_us(1000*1000);
        led_button_r_toggle();

        if(countdown == 0){
            io_powerdown();
        }
    }
}

