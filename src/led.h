/*
    Copyright 2016 fishpepper <AT> gmail.com

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

#ifndef LED_H_
#define LED_H_

#include <libopencm3/stm32/gpio.h>
#include "config.h"

void led_init(void);

#define led_backlight_on()     { gpio_set(LED_BACKLIGHT_GPIO, LED_BACKLIGHT_PIN); }
#define led_backlight_off()    { gpio_clear(LED_BACKLIGHT_GPIO, LED_BACKLIGHT_PIN); }
#define led_backlight_toggle() { gpio_toggle(LED_BACKLIGHT_GPIO, LED_BACKLIGHT_PIN); }

#define led_button_r_on()      { gpio_set(LED_BUTTON_R_GPIO, LED_BUTTON_R_PIN); }
#define led_button_r_off()     { gpio_clear(LED_BUTTON_R_GPIO, LED_BUTTON_R_PIN); }
#define led_button_r_toggle()  { gpio_toggle(LED_BUTTON_R_GPIO, LED_BUTTON_R_PIN); }

#define led_button_l_on()      { gpio_set(LED_BUTTON_L_GPIO, LED_BUTTON_L_PIN); }
#define led_button_l_off()     { gpio_clear(LED_BUTTON_L_GPIO, LED_BUTTON_L_PIN); }
#define led_button_l_toggle()  { gpio_toggle(LED_BUTTON_L_GPIO, LED_BUTTON_L_PIN); }

#endif  // LED_H_
