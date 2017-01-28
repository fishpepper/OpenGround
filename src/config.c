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

#include "config.h"
#include  "stm32f0xx_rcc.h"

void config_init(void) {
    config_detect_hw_revision();
}

// autodetect hw revision works as follows:
// tgy evolution has a pulldown on RF0 (=PE.10)
void config_detect_hw_revision(void) {
    // enable peripheral clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

    // init PE.10 as intput with PULLUP
    GPIO_InitTypeDef gpio_init;
    GPIO_StructInit(&gpio_init);
    gpio_init.GPIO_Pin   = GPIO_Pin_10;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &gpio_init);

    // now we can check for the pullwon resistor:
    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_10) == 0) {
        // pulled down -> tgy evolution
        config_hw_revision = CONFIG_HW_REVISION_EVOLUTION;
    } else {
        // no pulldown -> high -> i6s
        config_hw_revision = CONFIG_HW_REVISION_I6S;
    }
}

