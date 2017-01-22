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
    GPIO_StructInit(&gpio_init);

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


void io_test_prepare(void){
    //sett all ios to input
    GPIO_InitTypeDef gpio_init;
    GPIO_StructInit(&gpio_init);
    gpio_init.GPIO_Pin   = 0xFFFF;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_Init(GPIOA, &gpio_init);
    GPIO_Init(GPIOB, &gpio_init);
    GPIO_Init(GPIOC, &gpio_init);
    GPIO_Init(GPIOD, &gpio_init);
    GPIO_Init(GPIOE, &gpio_init);
    GPIO_Init(GPIOF, &gpio_init);
}

//show status of all gpios on screen
void io_test(void){
    uint32_t i,p;
    while(1){
        console_clear();
        debug("GPIO TEST\n\n");
        debug("       0123456789ABCDEF\n");
        for(p=0; p<6; p++){
            debug("GPIO");
            debug_putc('A'+p);
            debug("  ");
            for(i=0; i<16; i++){
                if (GPIO_ReadInputDataBit(((GPIO_TypeDef *) (GPIOA_BASE + p*0x00000400)), (1<<i))){
                    debug_putc('1');
                }else{
                    debug_putc('0');
                }
            }
            debug_put_newline();
        }
        debug_flush();
        delay_ms(50);
    }
}

void io_powerdown(void){
    POWERDOWN_GPIO->BRR = (POWERDOWN_PIN);
}

