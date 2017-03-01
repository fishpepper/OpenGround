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
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

config_hw_revision_t config_hw_revision;

void config_init(void) {
    config_detect_hw_revision();
}

// autodetect hw revision works as follows:
// tgy evolution has a pulldown on RF0 (=PE.10)
void config_detect_hw_revision(void) {
    // enable peripheral clock
    rcc_periph_clock_enable(GPIO_RCC(HW_REVISION_GPIO));

    // init PE.10 as intput with PULLUP
    gpio_mode_setup(HW_REVISION_GPIO, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, HW_REVISION_PIN);
    gpio_set_output_options(HW_REVISION_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, HW_REVISION_PIN);

    // now we can check for the pullwon resistor:
    if (gpio_get(HW_REVISION_GPIO, HW_REVISION_PIN) == 0) {
        // pulled down -> tgy evolution
        config_hw_revision = CONFIG_HW_REVISION_EVOLUTION;
    } else {
        // no pulldown -> high -> i6s
        config_hw_revision = CONFIG_HW_REVISION_I6S;
    }
}


