#include "main.h"
#include "stm32f0xx_gpio.h"
#include "config.h"
#include "delay.h"
#include "led.h"
#include "io.h"
#include "lcd.h"
#include "screen.h"
#include "console.h"
#include "adc.h"
#include "sound.h"

int main(void) {
    delay_init();
    led_init();
    io_init();

    led_on();
    lcd_init();
    screen_init();
    console_init();
    debug_init();
    adc_init();
    sound_init();


    led_on();
    uint8_t countdown = 10;
    debug("main: starting loop\n"); debug_flush();
    while(1){
#if 1
        console_clear();
        debug("ADC TEST  BAT: ");
        debug_put_fixed2(adc_get_battery_voltage());
        debug(" V\n");
        uint32_t i;
        adc_process();
        for(i=0; i<ADC_CHANNEL_COUNT; i++){
            debug_put_uint8(i+0); debug_putc('=');
            debug_put_hex16(adc_get_channel(i+0));
            if (i&1){
                debug_put_newline();
            }else{
                debug(" ");
            }
        }
        console_render();

        delay_us(10*1000);
#else
        debug("power down in ");
        debug_put_uint8(countdown--);
        debug("s\n");

        console_render();
        delay_us(1000*1000);
        led_button_r_toggle();

        if(countdown == 0){
            io_powerdown();
        }
#endif
    }
}

