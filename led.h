#ifndef __LED__H_
#define __LED__H_
#include "stm32f0xx_gpio.h"
#include "config.h"

void led_init(void);

#define led_backlight_on()   { LED_BACKLIGHT_GPIO->BSRR = (LED_BACKLIGHT_PIN); }
#define led_backlight_off()  { LED_BACKLIGHT_GPIO->BRR = (LED_BACKLIGHT_PIN); }
#define led_backlight_toggle(){ LED_BACKLIGHT_GPIO->BSRR = (LED_BACKLIGHT_GPIO->ODR ^ LED_BACKLIGHT_PIN) | (LED_BACKLIGHT_PIN << 16);}

#define led_button_r_on() { LED_BUTTON_R_GPIO->BSRR = (LED_BUTTON_R_PIN); }
#define led_button_r_off()  { LED_BUTTON_R_GPIO->BRR = (LED_BUTTON_R_PIN); }
#define led_button_r_toggle(){ LED_BUTTON_R_GPIO->BSRR = (LED_BUTTON_R_GPIO->ODR ^ LED_BUTTON_R_PIN) | (LED_BUTTON_R_PIN << 16);}

#define led_button_l_on() { LED_BUTTON_L_GPIO->BSRR = (LED_BUTTON_L_PIN); }
#define led_button_l_off()  { LED_BUTTON_L_GPIO->BRR = (LED_BUTTON_L_PIN); }
#define led_button_l_toggle(){ LED_BUTTON_L_GPIO->BSRR = (LED_BUTTON_L_GPIO->ODR ^ LED_BUTTON_L_PIN) | (LED_BUTTON_L_PIN << 16);}


#endif //__LED__H_

