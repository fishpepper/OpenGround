/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

   author: fishpepper <AT> gmail.com
*/

#include "io.h"
#include "debug.h"
#include "config.h"
#include "console.h"
#include "led.h"
#include "delay.h"
#include  "stm32f0xx_rcc.h"


void io_init(void) {
    debug("io: init\n"); debug_flush();
    io_init_gpio();
}

void io_init_gpio(void) {
    GPIO_InitTypeDef gpio_init;

    // clock enable
    RCC_AHBPeriphClockCmd(POWERDOWN_GPIO_CLK, ENABLE);

    //set high:
    POWERDOWN_GPIO->BSRR = (POWERDOWN_PIN);

    //set pin as output
    gpio_init.GPIO_Pin   = POWERDOWN_PIN;
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(POWERDOWN_GPIO, &gpio_init);
}

void io_powerdown(void){
    POWERDOWN_GPIO->BRR = (POWERDOWN_PIN);
}

void io_powerdown_test(void){
    uint8_t countdown = 10;
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

