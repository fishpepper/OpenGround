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

#include "led.h"
#include "macros.h"
#include <libopencm3/stm32/rcc.h>

void led_init(void) {
    // peripheral clocks enable
    rcc_periph_clock_enable(GPIO_RCC(LED_BACKLIGHT_GPIO));
    rcc_periph_clock_enable(GPIO_RCC(LED_BUTTON_R_GPIO));
    rcc_periph_clock_enable(GPIO_RCC(LED_BUTTON_L_GPIO));

    // set led pin as output
    gpio_mode_setup(LED_BACKLIGHT_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_BACKLIGHT_PIN);
    gpio_set_output_options(LED_BACKLIGHT_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, LED_BACKLIGHT_PIN);

    // left button led
    gpio_mode_setup(LED_BUTTON_L_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_BUTTON_L_PIN);
    gpio_set_output_options(LED_BUTTON_L_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, LED_BUTTON_L_PIN);

    // right button led
    gpio_mode_setup(LED_BUTTON_R_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_BUTTON_R_PIN);
    gpio_set_output_options(LED_BUTTON_R_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, LED_BUTTON_R_PIN);

    led_button_l_on();
    led_button_r_on();
    led_backlight_off();
}

