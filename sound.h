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

#ifndef SOUND_H_
#define SOUND_H_

#include <stdint.h>

#include "stm32f0xx.h"

volatile __IO uint32_t sound_tone_duration;

void sound_enqueue(uint32_t freq, uint32_t duration_ms);

void sound_init(void);
void sound_set_frequency(uint32_t freq);
void sound_play_click(void);
static void sound_init_rcc(void);
static void sound_init_gpio(void);
void sound_handle_playback(void);

typedef struct {
    uint16_t frequency;
    uint16_t duration_ms;
} tone_t;

#endif  // SOUND_H_
