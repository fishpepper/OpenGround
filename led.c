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
#include "led.h"
#include  "stm32f0xx_rcc.h"

void led_init(void) {
    GPIO_InitTypeDef gpio_init;
    GPIO_StructInit(&gpio_init);

    // clock enable
    RCC_AHBPeriphClockCmd(LED_BACKLIGHT_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(LED_BUTTON_R_GPIO_CLK, ENABLE);

    //set led pin as output
    gpio_init.GPIO_Pin   = LED_BACKLIGHT_PIN;
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(LED_BACKLIGHT_GPIO, &gpio_init);

    gpio_init.GPIO_Pin   = LED_BUTTON_R_PIN | LED_BUTTON_L_PIN;
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(LED_BUTTON_R_GPIO, &gpio_init);

}

