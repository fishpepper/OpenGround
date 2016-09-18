#ifndef __TIMEOUT_H__
#define __TIMEOUT_H__
#include <stdint.h>
#include "stm32f0xx.h"

void timeout_init(void);
void timeout_set(__IO uint32_t ms);
void timeout_set_100us(__IO uint32_t hus);

uint8_t timeout_timed_out(void);
void timeout_delay_ms(uint32_t timeout);
uint32_t timeout_time_remaining(void);

void SysTick_Handler(void);

#endif
