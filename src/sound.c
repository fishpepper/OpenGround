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


#include "sound.h"
#include "config.h"
#include "debug.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/gpio.h>


#define SOUND_QUEUE_SIZE 10
volatile __IO uint32_t sound_tone_duration;
static tone_t sound_queue[SOUND_QUEUE_SIZE];
uint32_t sound_queue_state;


void sound_init(void) {
    debug("sound: init\n"); debug_flush();
    sound_init_rcc();
    sound_init_gpio();

    /*uint32_t i;
    for (i = 1; i < 10; i++) {
        sound_set_frequency(i*100);
        delay_us(1000*1000);
    }*/
    sound_queue_state = 0;
    sound_set_frequency(0);
/*
    sound_queue[0].frequency   = 500;
    sound_queue[0].duration_ms = 80;
    sound_queue[1].frequency   = 890;
    sound_queue[1].duration_ms = 80;
    sound_queue[2].frequency   = 1000;
    sound_queue[2].duration_ms = 80;
    sound_queue[3].frequency   = 0;
    sound_queue[3].duration_ms = 0;
    sound_queue_state = 1;
*/
    sound_tone_duration = 0;
}

void sound_play_bind(void) {
    sound_queue[0].frequency   = 2000;
    sound_queue[0].duration_ms = 100;
    sound_queue[1].frequency   = 1000;
    sound_queue[1].duration_ms = 100;
    sound_queue[2].duration_ms = 0;
    sound_queue_state = 1;
}

void sound_play_click(void) {
    sound_queue[0].frequency   = 20000;
    sound_queue[0].duration_ms = 80;
    sound_queue[1].duration_ms = 0;
    sound_queue_state = 1;
}

void sound_play_low_time(void) {
    sound_queue[0].frequency   = 4000;
    sound_queue[0].duration_ms = 300;
    sound_queue[1].duration_ms = 0;
    sound_queue_state = 1;
}

static void sound_init_rcc(void) {
    // enable peripheral clock
    rcc_periph_clock_enable(GPIO_RCC(SPEAKER_GPIO));

    // timer1 clock
    rcc_periph_clock_enable(RCC_TIM1);
}

static void sound_init_gpio(void) {
    // set all gpio directions to output
    gpio_mode_setup(SPEAKER_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPEAKER_PIN);
    gpio_mode_setup(SPEAKER_GPIO, GPIO_MODE_AF, GPIO_PUPD_NONE, SPEAKER_PIN);
    // connect TIM1 pins to AF2 on gpio
    gpio_set_af(SPEAKER_GPIO, GPIO_AF2, SPEAKER_PIN);
}

#if 0

/* Enable timer clock. */
rcc_peripheral_enable_clock(reg, en);

/* Reset TIM1 peripheral */
timer_reset(timer_peripheral);



#endif

void sound_set_frequency(uint32_t freq) {
    uint32_t prescaler, period;

    if (freq <= 200) {
        // switch off pwm
        timer_disable_oc_output(TIM1, TIM_OC1);
        return;
    }

    // reset TIMx peripheral
    timer_reset(TIM1);

    // roughly factor into 16-bit
    period    = (rcc_apb1_frequency / 1) / freq;
    prescaler = (period / 65536) + 1;
    period    = (period / prescaler);



    // Set the timers global mode to:
    // - use no divider
    // - alignment edge
    // - count direction up
    timer_set_mode(TIM1,
                   TIM_CR1_CKD_CK_INT,
                   TIM_CR1_CMS_EDGE,
                   TIM_CR1_DIR_UP);

    timer_set_prescaler(TIM1, prescaler - 1);
    timer_set_repetition_counter(TIM1, 0);

    timer_enable_preload(TIM1);
    timer_continuous_mode(TIM1);
    timer_set_period(TIM1, period - 1);

    // start with disabled pwm output
    timer_disable_oc_output(TIM1, TIM_OC1);

    // configure output mode
    timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
    timer_set_oc_value(TIM1, TIM_OC1, 0);
    // set period for 50/50 duty cycle
    timer_set_oc_value(TIM1, TIM_OC1, period / 2);
    // enable pwm output
    timer_enable_oc_output(TIM1, TIM_OC1);

    // start timer
    timer_enable_counter(TIM1);
}

static void sound_play_sample(tone_t *tone) {
    uint8_t i;
    // add this sound sample to the playback queue:
    for (i = 0; i < SOUND_QUEUE_SIZE; i++) {
        sound_queue[i] = *tone;
        if ((tone->duration_ms == 0) && (tone->frequency == 0)) {
            // done, this was the last sample
            break;
        } else {
            tone++;
        }
    }
    sound_queue_state = 0;
}

void sound_handle_playback(void) {
    if (sound_queue_state == 0) {
        // off, return
        return;
    }

    if (sound_tone_duration == 0) {
        // next sample
        uint32_t id = sound_queue_state - 1;
        if ((id == SOUND_QUEUE_SIZE) || (sound_queue[id].duration_ms == 0)) {
            // no more samples, switch off:
            sound_set_frequency(0);
            sound_queue_state = 0;
        } else {
            // fetch next sample
            sound_tone_duration = 10*sound_queue[id].duration_ms;
            sound_set_frequency(sound_queue[id].frequency);
            sound_queue_state++;
        }

    } else {
        sound_tone_duration--;
    }
}
