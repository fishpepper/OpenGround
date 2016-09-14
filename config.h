#ifndef __PIN_CONFIG_H__
#define __PIN_CONFIG_H__
#include "main.h"

//BACKGROUND LED IS PF.3 = PIN 22

#define LED_GPIO                  GPIOF
#define LED_GPIO_CLK              RCC_AHBPeriph_GPIOF
#define LED_PIN                   GPIO_Pin_3

#endif // __PIN_CONFIG_H__
