#ifndef __LED__H_
#define __LED__H_
#include "stm32f0xx_gpio.h"
#include "config.h"

void led_init(void);

#define led_on()   { LED_GPIO->BSRR = (LED_PIN); }
#define led_off()  { LED_GPIO->BRR = (LED_PIN); }
#define led_toggle(){ LED_GPIO->BSRR = (LED_GPIO->ODR ^ LED_PIN) | (LED_PIN << 16);}

#endif //__LED__H_

