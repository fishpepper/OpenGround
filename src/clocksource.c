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

#include "clocksource.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>

uint32_t rcc_timer_frequency;

void clocksource_init(void) {
    // set clock source
    clocksource_hse_in_8_out_48();
}

void clocksource_hse_in_8_out_48(void) {
    // see
    // https://www.mikrocontroller.net/attachment/322047/Clock_Control.png
    // or RM00091 p. 98

    // enable internal high-speed oscillator
    rcc_osc_on(RCC_HSI);
    rcc_wait_for_osc_ready(RCC_HSI);

    // Select HSI as SYSCLK source.
    rcc_set_sysclk_source(RCC_CFGR_SW_HSI);

    // Enable external high-speed oscillator 8MHz
    rcc_osc_on(RCC_HSE);
    rcc_wait_for_osc_ready(RCC_HSE);
    rcc_set_sysclk_source(RCC_CFGR_SW_HSE);

    // set prescalers for AHB, ADC, ABP1, ABP2.
    // Do this before touching the PLL
    rcc_set_hpre(RCC_CFGR_HPRE_NODIV);      // 48Mhz (max 72)
    rcc_set_ppre(RCC_CFGR_PPRE_DIV2);       // 24Mhz (max 36)

    // sysclk runs with 48MHz -> 1 waitstates.
    // * 0WS from 0-24MHz
    // * 1WS from 24-48MHz
    // * 2WS from 48-72MHz
    flash_set_ws(FLASH_ACR_LATENCY_1WS);

    // set the PLL multiplication factor to 6
    // pll source is hse
    RCC_CFGR |= RCC_CFGR_PLLSRC;
    // pll prediv = 1
    rcc_set_prediv(RCC_CFGR2_PREDIV_NODIV);
    // 8MHz (external) * 6 (multiplier) = 48MHz
    rcc_set_pll_multiplication_factor(RCC_CFGR_PLLMUL_MUL6);

    // enable PLL oscillator and wait for it to stabilize.
    rcc_osc_on(RCC_PLL);
    rcc_wait_for_osc_ready(RCC_PLL);

    // select PLL as SYSCLK source.
    rcc_set_sysclk_source(RCC_PLL);

    // set the peripheral clock frequencies used */
    rcc_ahb_frequency  = 48000000;
    rcc_apb1_frequency = 24000000;

    // When PPRE is set to something != NODIV
    // TIM input clock is apb clkspeed*2 (see RM00091 p98)
    rcc_timer_frequency = 2*rcc_apb1_frequency;
}

