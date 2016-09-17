#ifndef __PIN_CONFIG_H__
#define __PIN_CONFIG_H__
#include "main.h"

// see http://fishpepper.de/2016/09/15/openground-part-1-components-pinout/
// for pin mappings on flysky fs-i6s

// LEDS
#define LED_BACKLIGHT_GPIO                  GPIOF
#define LED_BACKLIGHT_GPIO_CLK              RCC_AHBPeriph_GPIOF
#define LED_BACKLIGHT_PIN                   GPIO_Pin_3

//L BUTTON BLUE = PD10
//R BUTTON BLUE = PD11
#define LED_BUTTON_R_GPIO      GPIOD
#define LED_BUTTON_R_GPIO_CLK  RCC_AHBPeriph_GPIOD
#define LED_BUTTON_R_PIN       GPIO_Pin_11

// POWERDOWN TRIGGER
#define POWERDOWN_GPIO      GPIOB
#define POWERDOWN_GPIO_CLK  RCC_AHBPeriph_GPIOB
#define POWERDOWN_PIN       GPIO_Pin_15


// LCD
// data lines
#define LCD_DATA_GPIO        GPIOE
#define LCD_DATA_GPIO_CLK    RCC_AHBPeriph_GPIOE
// RW
#define LCD_RW_GPIO          GPIOB
#define LCD_RW_GPIO_CLK      RCC_AHBPeriph_GPIOB
#define LCD_RW_PIN           GPIO_Pin_5
// RST
#define LCD_RST_GPIO         GPIOB
#define LCD_RST_GPIO_CLK     RCC_AHBPeriph_GPIOB
#define LCD_RST_PIN          GPIO_Pin_4
// RS
#define LCD_RS_GPIO          GPIOB
#define LCD_RS_GPIO_CLK      RCC_AHBPeriph_GPIOB
#define LCD_RS_PIN           GPIO_Pin_3
// RD
#define LCD_RD_GPIO          GPIOD
#define LCD_RD_GPIO_CLK      RCC_AHBPeriph_GPIOD
#define LCD_RD_PIN           GPIO_Pin_7
// CS
#define LCD_CS_GPIO          GPIOD
#define LCD_CS_GPIO_CLK      RCC_AHBPeriph_GPIOD
#define LCD_CS_PIN           GPIO_Pin_2


#endif // __PIN_CONFIG_H__
