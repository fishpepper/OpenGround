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

#include "sound.h"
#include "config.h"
#include "debug.h"
#include  "stm32f0xx_rcc.h"
#include  "stm32f0xx_gpio.h"
#include  "stm32f0xx_tim.h"

void sound_init(void) {
    debug("sound: init\n"); debug_flush();
    sound_init_rcc();
    sound_init_gpio();

    uint32_t i;
    for(i=1; i<10; i++){
        sound_set_frequency(i*100);
        delay_us(1000*1000);
    }
    sound_set_frequency(0);
}

static void sound_init_rcc(void) {
    // enable all peripheral clocks:
    RCC_AHBPeriphClockCmd(SPEAKER_GPIO_CLK, ENABLE);
    // timer1 clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
}

static void sound_init_gpio(void) {
    GPIO_InitTypeDef gpio_init;

    // set all gpio directions to output
    gpio_init.GPIO_Pin   = SPEAKER_PIN;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(SPEAKER_GPIO, &gpio_init);

    //connect TIM1 pins to AF
    GPIO_PinAFConfig(SPEAKER_GPIO, SPEAKER_PIN_SOURCE, GPIO_AF_2);
}

void sound_set_frequency(uint32_t freq){
    uint32_t prescaler, period;
    TIM_TimeBaseInitTypeDef  tim_init;
    TIM_OCInitTypeDef  tim_oc_init;

    if (freq <= 200){
        //switch off pwm
        TIM_CtrlPWMOutputs(TIM1, DISABLE);
    }

    //roughly factor into 16-bit
    period    = (SystemCoreClock / 1) / freq;
    prescaler = (period / 65536) + 1;
    period    = (period / prescaler);

    //time base config
    TIM_TimeBaseStructInit(&tim_init);
    tim_init.TIM_Prescaler     = prescaler - 1;
    tim_init.TIM_Period        = period - 1;
    tim_init.TIM_ClockDivision = 0;
    tim_init.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &tim_init);

    debug_put_hex32(SystemCoreClock); debug_flush(); debug_put_newline();
    debug("presc = "); debug_put_uint16(prescaler);
    debug("\nperi = "); debug_put_uint16(period); debug_flush();

    //PWM1 Mode configuration: Channel1
    TIM_OCStructInit(&tim_oc_init);
    tim_oc_init.TIM_OCMode      = TIM_OCMode_PWM1;
    tim_oc_init.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc_init.TIM_Pulse       = period / 2; // 50/50 duty
    tim_oc_init.TIM_OCPolarity  = TIM_OCPolarity_High;

    TIM_OC1Init(TIM1, &tim_oc_init);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

    //enable counter
    TIM_Cmd(TIM1, ENABLE);

    //main Output enable
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

