#include "clocksource.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>


void clocksource_init(void) {
    // set clock source
    clocksource_hse_in_8_out_48();
}

void clocksource_hse_in_8_out_48(void) {
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
    rcc_set_ppre(RCC_CFGR_PPRE_DIV2);       // 24Mhz (max 36)a
    rcc_set_prediv(RCC_CFGR2_PREDIV_NODIV);  

    // sysclk runs with 48MHz -> 1 waitstates.
    // * 0WS from 0-24MHz
    // * 1WS from 24-48MHz
    // * 2WS from 48-72MHz
    flash_set_ws(FLASH_ACR_LATENCY_1WS);

    // set the PLL multiplication factor to 6
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
}

