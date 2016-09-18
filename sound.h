#ifndef __SOUND_H__
#define __SOUND_H__
#include <stdint.h>

void sound_init(void);
void sound_set_frequency(uint32_t freq);

static void sound_init_rcc(void);
static void sound_init_gpio(void);

#endif // __SOUND_H__
