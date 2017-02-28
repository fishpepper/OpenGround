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

#include "wdt.h"
#include "debug.h"
#include "delay.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/iwdg.h>

void wdt_init(void) {
    // detect resets from wdt
    if (RCC_CSR & RCC_CSR_IWDGRSTF) {
        debug("hal_wdt: watchdog reset detected\n"); debug_flush();
        RCC_CSR &= ~(RCC_CSR_IWDGRSTF);
    }

    // set wdg timeout to roughly 1000ms(varies due to LSI freq dispersion)
    iwdg_set_period_ms(1000);

    // enable IWDG(the LSI oscillator will be enabled by hardware)
    iwdg_start();
}

inline void wdt_reset(void) {
    // reset wdt
     iwdg_reset();
}

