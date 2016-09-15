#include "main.h"
#include "stm32f0xx_gpio.h"
#include "config.h"
#include "delay.h"
#include "led.h"
#include "lcd.h"
#include "screen.h"

int main(void) {
    delay_init();
    led_init();
   
    led_on(); 
    lcd_init();
    screen_init();
    

    led_on();
    while(1){
	led_toggle();
	delay_us(1000*1000);
    }
}

