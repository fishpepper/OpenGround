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

#ifndef CONFIG_H_
#define CONFIG_H_
#include "main.h"

// see http://fishpepper.de/2016/09/15/openground-part-1-components-pinout/
// for pin mappings on flysky fs-i6s

// LEDS
#define LED_BACKLIGHT_GPIO                  GPIOF
#define LED_BACKLIGHT_GPIO_CLK              RCC_AHBPeriph_GPIOF
#define LED_BACKLIGHT_PIN                   GPIO_Pin_3

// L BUTTON BLUE = PD10
// R BUTTON BLUE = PD11
#define LED_BUTTON_L_GPIO      GPIOD
#define LED_BUTTON_L_GPIO_CLK  RCC_AHBPeriph_GPIOD
#define LED_BUTTON_L_PIN       GPIO_Pin_10
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

// speaker
#define SPEAKER_GPIO         GPIOA
#define SPEAKER_GPIO_CLK     RCC_AHBPeriph_GPIOA
#define SPEAKER_PIN          GPIO_Pin_8
#define SPEAKER_PIN_SOURCE   8

// irq priorities
#define NVIC_PRIO_FRSKY      0
#define NVIC_PRIO_SYSTICK    1
#define NVIC_PRIO_TOUCH      3

// touch
#define TOUCH_FT6236_I2C_ADDRESS      0x70
#define TOUCH_I2C                     I2C1
#define TOUCH_I2C_CLK                 RCC_APB1Periph_I2C1
#define TOUCH_I2C_GPIO_CLK            RCC_AHBPeriph_GPIOB
#define TOUCH_I2C_GPIO                GPIOB
#define TOUCH_I2C_SDA_PIN             GPIO_Pin_9
#define TOUCH_I2C_SCL_PIN             GPIO_Pin_8

#define TOUCH_RESET_GPIO              GPIOA
#define TOUCH_RESET_GPIO_CLK          RCC_AHBPeriph_GPIOA
#define TOUCH_RESET_PIN               GPIO_Pin_15

#define TOUCH_INT_GPIO                GPIOC
#define TOUCH_INT_GPIO_CLK            RCC_AHBPeriph_GPIOC
#define TOUCH_INT_PIN                 GPIO_Pin_12

#define TOUCH_INT_EXTI_SOURCE         EXTI_PortSourceGPIOC
#define TOUCH_INT_EXTI_SOURCE_PIN     EXTI_PinSource12
#define TOUCH_INT_EXTI_SOURCE_LINE    EXTI_Line12
#define TOUCH_INT_EXTI_IRQN           EXTI4_15_IRQn

// cc2500 module connection
// SI = SDIO
// SCK = SCK
// MISO = GD1
// GDO2 = GDO2
// GDO0 = RF1
// CSN = SCS
// PA_EN = TX = TXW
// LNA_EN = RX = RXW
#define CC2500_SPI_GPIO             GPIOE
// LABELED SCK
#define CC2500_SPI_SCK_PIN          GPIO_Pin_13
#define CC2500_SPI_SCK_PINSOURCE    GPIO_PinSource13
// LABELED SPIO
#define CC2500_SPI_MOSI_PIN         GPIO_Pin_15
#define CC2500_SPI_MOSI_PINSOURCE   GPIO_PinSource15
// LABELED GIO1
#define CC2500_SPI_MISO_PIN         GPIO_Pin_14
#define CC2500_SPI_MISO_PINSOURCE   GPIO_PinSource14
// LABELED RF2
#define CC2500_SPI_CSN_PIN          GPIO_Pin_12
#define CC2500_SPI                  SPI1
#define CC2500_SPI_CLK              RCC_APB2Periph_SPI1
#define CC2500_SPI_GPIO_CLK         RCC_AHBPeriph_GPIOE
#define CC2500_SPI_DMA_CLOCK        RCC_AHBPeriph_DMA1
#define CC2500_SPI_TX_DMA_CHANNEL   DMA1_Channel3
#define CC2500_SPI_TX_DMA_TC_FLAG   DMA1_FLAG_TC3
#define CC2500_SPI_RX_DMA_CHANNEL   DMA1_Channel2
#define CC2500_SPI_RX_DMA_TC_FLAG   DMA1_FLAG_TC2

// LABELED RF1
#define CC2500_GDO1_PIN            GPIO_Pin_11
#define CC2500_GDO1_GPIO           GPIOE
#define CC2500_GDO1_GPIO_CLK       RCC_AHBPeriph_GPIOE

// LABELED RX-W
#define CC2500_LNA_GPIO     GPIOE
#define CC2500_LNA_GPIO_CLK RCC_AHBPeriph_GPIOE
#define CC2500_LNA_PIN      GPIO_Pin_9

// LABELED TX_W
#define CC2500_PA_GPIO     GPIOE
#define CC2500_PA_GPIO_CLK RCC_AHBPeriph_GPIOE
#define CC2500_PA_PIN      GPIO_Pin_8

// LABELED GIO2
#define CC2500_GDO2_GPIO           GPIOB
#define CC2500_GDO2_PIN            GPIO_Pin_2
#define CC2500_GDO2_GPIO_CLK       RCC_AHBPeriph_GPIOB

// BUTTONS
#define BUTTON_POWER_BOTH_GPIO        GPIOB
#define BUTTON_POWER_BOTH_GPIO_CLK    RCC_AHBPeriph_GPIOB
#define BUTTON_POWER_BOTH_PIN         GPIO_Pin_14

// BUTTON_BACK_RIGHT = PA10
// BUTTON_BACK_LEFT  = PA9


#endif  // CONFIG_H_
