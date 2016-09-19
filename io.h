#ifndef __IO_H__
#define __IO_H__
#include <stdint.h>
#include "stm32f0xx_gpio.h"

void io_init(void);
void io_init_gpio(void);
void io_powerdown(void);
void io_powerdown_test(void);
#endif // __IO_H__
