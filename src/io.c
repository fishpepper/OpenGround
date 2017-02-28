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


#include "io.h"
#include "debug.h"
#include "config.h"
#include "console.h"
#include "led.h"
#include "delay.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>


void io_init(void) {
    debug("io: init\n"); debug_flush();
    io_init_gpio();
}

void io_init_gpio(void) {
    // enable clocks
    rcc_periph_clock_enable(POWERDOWN_GPIO_CLK);
    rcc_periph_clock_enable(BUTTON_POWER_BOTH_GPIO_CLK);

    // set high:
    gpio_set(POWERDOWN_GPIO, POWERDOWN_PIN);

    // set powerdown trigger pin as output
    gpio_mode_setup(
        POWERDOWN_GPIO,
        GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE,
        POWERDOWN_PIN);

    // set buttons as input:
    gpio_mode_setup(
        BUTTON_POWER_BOTH_GPIO,
        GPIO_MODE_INPUT,
        GPIO_PUPD_PULLUP,
        BUTTON_POWER_BOTH_PIN);
}


void io_test_prepare(void) {
    // set all ios to input
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, 0xFFFF);
    gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, 0xFFFF);
    gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, 0xFFFF);
    gpio_mode_setup(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, 0xFFFF);
    gpio_mode_setup(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, 0xFFFF);
    gpio_mode_setup(GPIOF, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, 0xFFFF);
}

uint32_t io_powerbutton_pressed(void) {
    return(gpio_get(BUTTON_POWER_BOTH_GPIO, BUTTON_POWER_BOTH_PIN) == 0);
}

// show status of all gpios on screen
void io_test(void) {
    uint32_t i, p;

    while (1) {
        console_clear();
        debug("GPIO TEST\n\n");
        debug("       FEDCBA9876543210\n");
        for (p = 0; p < 6; p++) {
            debug("GPIO");
            debug_putc('A'+p);
            debug("  ");
            for (i = 0; i < 16; i++) {
                if (gpio_get((GPIO_PORT_A_BASE + p*0x00000400), (1 << (15-i)))) {
                    debug_putc('1');
                } else {
                    debug_putc('0');
                }
            }
            debug_put_newline();
        }
        debug_flush();
        delay_ms(50);
    }
}

void io_powerdown(void) {
    gpio_clear(POWERDOWN_GPIO, POWERDOWN_PIN);

    // system is powered off now...
    while (1) { }
}

