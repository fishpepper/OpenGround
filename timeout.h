#ifndef __TIMEOUT_H__
#define __TIMEOUT_H__
#include <stdint.h>
#include "stm32f0xx.h"

void timeout_init(void);
//void timeout_set(__IO uint32_t ms);
#define timeout_set(x) timeout_set_100us(10*(x));
void timeout_set_100us(__IO uint32_t hus);
void timeout2_set_100us(__IO uint32_t hus);
void timeout2_delay_100us(uint16_t us);
uint8_t timeout_timed_out(void);
uint8_t timeout2_timed_out(void);
void timeout_delay_ms(uint32_t timeout);
uint32_t timeout_time_remaining(void);

void SysTick_Handler(void);

#endif
