#ifndef __SOUND_H__
#define __SOUND_H__
#include <stdint.h>
#include "stm32f0xx.h"

volatile __IO uint32_t sound_tone_duration;

void sound_enqueue(uint32_t freq, uint32_t duration_ms);

void sound_init(void);
void sound_set_frequency(uint32_t freq);

static void sound_init_rcc(void);
static void sound_init_gpio(void);
void sound_handle_playback(void);

typedef struct {
    uint16_t frequency;
    uint16_t duration_ms;
} tone_t;

#endif // __SOUND_H__
