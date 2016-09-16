#include "main.h"
#include "stm32f0xx_gpio.h"
#include "config.h"
#include "delay.h"
#include "led.h"
#include "lcd.h"
#include "screen.h"
#include "console.h"

int main(void) {
    delay_init();
    led_init();
   
    led_on(); 
    lcd_init();
    screen_init();
    console_init();
    debug_init();
    

    led_on();
    while(1){
        console_render();
	led_off();
	delay_us(10*1000);
        led_on();
        delay_us(1000*1000);
    }
}

