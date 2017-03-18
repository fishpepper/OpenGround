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


typedef enum {
  CONFIG_HW_REVISION_I6S = 0,
  CONFIG_HW_REVISION_EVOLUTION,
  CONFIG_HW_REVISION_SIZE
} config_hw_revision_t;

extern config_hw_revision_t config_hw_revision;
void config_init(void);
void config_detect_hw_revision(void);

// see http://fishpepper.de/2016/09/15/openground-part-1-components-pinout/
// for pin mappings on flysky fs-i6s


// revision selection button
#define HW_REVISION_GPIO        GPIOE
#define HW_REVISION_PIN         GPIO10

// LEDS
#define LED_BACKLIGHT_GPIO                  GPIOF
#define LED_BACKLIGHT_PIN                   GPIO3

// L BUTTON BLUE = PD10
// R BUTTON BLUE = PD11
#define LED_BUTTON_L_GPIO      GPIOD
#define LED_BUTTON_L_PIN       GPIO10
#define LED_BUTTON_R_GPIO      GPIOD
#define LED_BUTTON_R_PIN       GPIO11

// POWERDOWN TRIGGER
#define POWERDOWN_GPIO      GPIOB
#define POWERDOWN_PIN       GPIO15


// LCD
// data lines
#define LCD_DATA_GPIO        GPIOE
// RW
#define LCD_RW_GPIO          GPIOB
#define LCD_RW_PIN           GPIO5
// RST
#define LCD_RST_GPIO         GPIOB
#define LCD_RST_PIN          GPIO4
// RS
#define LCD_RS_GPIO          GPIOB
#define LCD_RS_PIN           GPIO3
// RD
#define LCD_RD_GPIO          GPIOD
#define LCD_RD_PIN           GPIO7
// CS
#define LCD_CS_GPIO          GPIOD
#define LCD_CS_PIN           GPIO2

// speaker
#define SPEAKER_GPIO         GPIOA
#define SPEAKER_PIN          GPIO8

// irq priorities
#define NVIC_PRIO_FRSKY      0*64
#define NVIC_PRIO_SYSTICK    1*64
#define NVIC_PRIO_TOUCH      3*64

// touch
#define TOUCH_FT6236_I2C_ADDRESS      (0x70>>1)
#define TOUCH_I2C                     I2C1
#define TOUCH_I2C_CLK                 RCC_I2C1
#define TOUCH_I2C_GPIO                GPIOB
#define TOUCH_I2C_SDA_PIN             GPIO9
#define TOUCH_I2C_SCL_PIN             GPIO8

#define TOUCH_RESET_GPIO              GPIOA
#define TOUCH_RESET_PIN               GPIO15

#define TOUCH_INT_GPIO                GPIOC
#define TOUCH_INT_PIN                 GPIO12

#define TOUCH_INT_EXTI_SOURCE         GPIOC
#define TOUCH_INT_EXTI_SOURCE_LINE    EXTI12
#define TOUCH_INT_EXTI_IRQN           NVIC_EXTI4_15_IRQ



#define ADC_DMA_CHANNEL           DMA_CHANNEL1
#define ADC_DMA_TC_FLAG           DMA_ISR_TCIF1
#define ADC_CHANNEL_COUNT 11

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
#define CC2500_SPI_SCK_PIN          GPIO13
// LABELED SPIO
#define CC2500_SPI_MOSI_PIN         GPIO15
// LABELED GIO1
#define CC2500_SPI_MISO_PIN         GPIO14
// LABELED RF2
#define CC2500_SPI_CSN_PIN          GPIO12
#define CC2500_SPI                  SPI1
#define CC2500_SPI                  SPI1
#define CC2500_SPI_DR               SPI1_DR
#define CC2500_SPI_CLK              RCC_SPI1
#define CC2500_SPI_DMA_CLOCK        RCC_DMA1
#define CC2500_SPI_TX_DMA_CHANNEL   DMA_CHANNEL3
#define CC2500_SPI_TX_DMA_TC_FLAG   DMA1_FLAG_TC3
#define CC2500_SPI_RX_DMA_CHANNEL   DMA_CHANNEL2
#define CC2500_SPI_RX_DMA_TC_FLAG   DMA1_FLAG_TC2

// LABELED RF1
#define CC2500_GDO1_PIN            GPIO11
#define CC2500_GDO1_GPIO           GPIOE

// LABELED RX-W
#define CC2500_LNA_GPIO     GPIOE
#define CC2500_LNA_PIN      GPIO9

// LABELED TX_W
#define CC2500_PA_GPIO     GPIOE
#define CC2500_PA_PIN      GPIO8

// LABELED GIO2
#define CC2500_GDO2_GPIO           GPIOB
#define CC2500_GDO2_PIN            GPIO2

// BUTTONS
#define BUTTON_POWER_BOTH_GPIO        GPIOB
#define BUTTON_POWER_BOTH_PIN         GPIO14

// BUTTON_BACK_RIGHT = PA10
// BUTTON_BACK_LEFT  = PA9

#define USB_GPIO GPIOA
#define USB_DP_PIN GPIO12
#define USB_DM_PIN GPIO11

#endif  // CONFIG_H_
