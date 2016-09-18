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

#include "timeout.h"
#include "debug.h"
#include "delay.h"
#include "led.h"
#include "sound.h"
#include "stm32f0xx_rcc.h"

volatile static __IO uint32_t timeout_100us;
volatile static __IO uint32_t timeout_100us_delay;

void timeout_init(void) {
    debug("timeout: init\n"); debug_flush();

    //configure 1ms sys tick:
    if (SysTick_Config(SystemCoreClock / 10000)){
        debug("timeout: failed to set systick timeout\n");
        debug_flush();
    }

    // set prio
    NVIC_SetPriority(SysTick_IRQn, NVIC_PRIO_SYSTICK);
    NVIC_EnableIRQ(SysTick_IRQn);
    //SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

    timeout_100us = 0;
    timeout_100us_delay = 0;
}

void timeout_set_100us(__IO uint32_t hus) {
    timeout_100us = hus;
}

uint8_t timeout_timed_out(void) {
    return (timeout_100us == 0);
}


// seperate ms delay function
void timeout_delay_ms(uint32_t timeout){
    timeout_100us_delay = 10*timeout;

    while(timeout_100us_delay > 0){
    }
}

void SysTick_Handler(void){
    if (timeout_100us != 0){
        timeout_100us--;
    }
    if (timeout_100us_delay != 0){
        timeout_100us_delay--;
    }
    sound_handle_playback();
}

uint32_t timeout_time_remaining(void) {
    return timeout_100us/10;
}
