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

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include <libopencm3/stm32/gpio.h>
#include "config.h"

// the screen itself is 128 x 64
#define LCD_WIDTH   128
#define LCD_HEIGHT   64

void lcd_init(void);
void lcd_send_data(const uint8_t *buf);
void lcd_powerdown(void);
void lcd_show_logo(void);

#define LCD_RW_HI()   { gpio_set(LCD_RW_GPIO, LCD_RW_PIN);  }
#define LCD_RW_LO()   { gpio_clear(LCD_RW_GPIO, LCD_RW_PIN); }

#define LCD_RST_HI()  { gpio_set(LCD_RST_GPIO, LCD_RST_PIN); }
#define LCD_RST_LO()  { gpio_clear(LCD_RST_GPIO, LCD_RST_PIN); }

#define LCD_RS_HI()   { gpio_set(LCD_RS_GPIO, LCD_RS_PIN); }
#define LCD_RS_LO()   { gpio_clear(LCD_RS_GPIO, LCD_RS_PIN); }

#define LCD_RD_HI()   { gpio_set(LCD_RD_GPIO, LCD_RD_PIN); }
#define LCD_RD_LO()   { gpio_clear(LCD_RD_GPIO, LCD_RD_PIN); }

#define LCD_CS_HI()   { gpio_set(LCD_CS_GPIO, LCD_CS_PIN); }
#define LCD_CS_LO()   { gpio_clear(LCD_CS_GPIO, LCD_CS_PIN); }

// #define LCD_DATA_SET(data) { GPIO_ODR(LCD_DATA_GPIO)
// = (GPIO_ODR(LCD_DATA_GPIO) & 0xFF00) | (data);}

#define LCD_DATA_SET(data) {((uint8_t __IO*)&GPIO_ODR(LCD_DATA_GPIO))[0] = (data); }


#define LCD_CMD_RESET            0xE2
#define LCD_CMD_BIAS_1_9         0xA2  // 1/9 bias
#define LCD_CMD_BIAS_1_7         0xA3  // 1/7 bias

#define LCD_CMD_SEG_NORMAL       0xA0
#define LCD_CMD_SEG_INVERSE      0xA1

#define LCD_CMD_COM_NORMAL       0xC0
#define LCD_CMD_COM_INVERSE      0xC8

#define LCD_CMD_REG_RATIO_000    0x20
#define LCD_CMD_REG_RATIO_001    0x21
#define LCD_CMD_REG_RATIO_010    0x22
#define LCD_CMD_REG_RATIO_011    0x23
#define LCD_CMD_REG_RATIO_100    0x24
#define LCD_CMD_REG_RATIO_101    0x25
#define LCD_CMD_REG_RATIO_110    0x26
#define LCD_CMD_REG_RATIO_111    0x27

#define LCD_CMD_EV               0x81
#define LCD_CMD_POWERCTRL_ALL_ON 0x2F

#define LCD_CMD_SET_STARTLINE    0x40
#define LCD_CMD_SET_PAGESTART    0xB0

#define LCD_CMD_SET_COL_LO       0x00
#define LCD_CMD_SET_COL_HI       0x10

#define LCD_CMD_DISPLAY_OFF      0xAE
#define LCD_CMD_DISPLAY_ON       0xAF

#define LCD_CMD_MODE_RAM         0xA4
#define LCD_CMD_MODE_ALLBLACK    0xA5


#endif  // LCD_H_

