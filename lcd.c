/*
    Copyright 2016 fishpepper <AT> gmail.com

    This program is free software: you can redistribute it and/ or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http:// www.gnu.org/licenses/>.

    author: fishpepper <AT> gmail.com
*/

// see:
// http://fishpepper.de/2016/09/15/openground-part-1-components-pinout/
// http://pdf.masters.com.pl/SITRONIX/ST7567.PDF
// http://edeca.net/wp/electronics/the-st7565-display-controller/
// https://github.com/opentx/opentx/blob/\
//         dbd8abbfe8343b5d7f542304d47e232140307b95/radio/src/targets/stock/lcd_driver.cpp

#include "lcd.h"
#include "wdt.h"
#include "delay.h"
#include  "stm32f0xx_rcc.h"

void lcd_init(void) {
    lcd_init_rcc();
    lcd_init_gpio();
    lcd_reset();
}


static void lcd_init_rcc(void) {
    // enable all peripheral clocks:
    RCC_AHBPeriphClockCmd(LCD_DATA_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(LCD_RW_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(LCD_RST_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(LCD_RS_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(LCD_RD_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(LCD_CS_GPIO_CLK, ENABLE);
}

static void lcd_init_gpio(void) {
    GPIO_InitTypeDef gpio_init;
    GPIO_StructInit(&gpio_init);

    // set all gpio directions to output
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    // set individual pins
    // data
    gpio_init.GPIO_Pin   = 0xFF;  // D0..D7
    // gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    // gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(LCD_DATA_GPIO, &gpio_init);

    // gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    // gpio_init.GPIO_Mode  = GPIO_Mode_OUT;

    // rw
    gpio_init.GPIO_Pin   = LCD_RW_PIN;
    GPIO_Init(LCD_RW_GPIO, &gpio_init);
    // rst
    gpio_init.GPIO_Pin   = LCD_RST_PIN;
    GPIO_Init(LCD_RST_GPIO, &gpio_init);
    // rs
    gpio_init.GPIO_Pin   = LCD_RS_PIN;
    GPIO_Init(LCD_RS_GPIO, &gpio_init);
    // rd
    gpio_init.GPIO_Pin   = LCD_RD_PIN;
    GPIO_Init(LCD_RD_GPIO, &gpio_init);
    // cs
    gpio_init.GPIO_Pin   = LCD_CS_PIN;
    GPIO_Init(LCD_CS_GPIO, &gpio_init);

    // set default pin levels:
    LCD_RST_LO();
    LCD_CS_HI();
    LCD_RW_LO();
    LCD_RD_HI();
    LCD_RS_HI();
}

static void lcd_write_command(uint8_t data) {
    // make sure write is high(disabled)
    LCD_CS_LO();
    // command mode
    LCD_RS_LO();
    //
    LCD_RW_LO();

    // write data to port d0...d7
    LCD_DATA_SET(data);
    // execute write
    LCD_RD_HI();
    LCD_RD_LO();
    LCD_RS_HI();

    // deselect device
    LCD_CS_HI();
}


void lcd_powerdown(void) {
    // switch display off
    lcd_write_command(LCD_CMD_DISPLAY_OFF);

    // all pixels on
    lcd_write_command(LCD_CMD_MODE_ALLBLACK);

    // wait
    wdt_reset();
    delay_ms(100);
    wdt_reset();
    delay_ms(100);
    wdt_reset();
    delay_ms(100);
}

static void lcd_reset(void) {
    // wait for voltages to be stable
    delay_ms(100);

    // trigger hw reset
    LCD_RST_LO();
    delay_us(10);  // at least 5us
    LCD_RST_HI();
    delay_us(1500);  // at least 5us?

    // send software reset command
    lcd_write_command(LCD_CMD_RESET);

    // switch display off
    lcd_write_command(LCD_CMD_DISPLAY_OFF);

    // do not set all pixels to black(..._ALLBLACK)
    lcd_write_command(LCD_CMD_MODE_RAM);

    // set to 1/ 7 bias:
    lcd_write_command(LCD_CMD_BIAS_1_7);  // opentx uses LCD_CMD_BIAS_1_9

    // set seg and com directions
    lcd_write_command(LCD_CMD_COM_NORMAL);  // INVERSE);
    // normal or inverted?
    lcd_write_command(LCD_CMD_SEG_INVERSE);  // NORMAL);

    // power control -> all boosters on
    lcd_write_command(LCD_CMD_POWERCTRL_ALL_ON);

    // regulator ratio
    lcd_write_command(LCD_CMD_REG_RATIO_011);  // opentx uses LCD_CMD_REG_RATIO_101 ?

    // set EV(contrast?), this is  2byte command
    lcd_write_command(LCD_CMD_EV);
    lcd_write_command(40);  // dynamic contrast 0...63

    // set startline to 0
    lcd_write_command(LCD_CMD_SET_STARTLINE + 0);
    // set start page to 0
    lcd_write_command(LCD_CMD_SET_PAGESTART + 0);

    // set col address of ram to 0
    lcd_write_command(LCD_CMD_SET_COL_LO + 0);
    lcd_write_command(LCD_CMD_SET_COL_HI + 0);

    // switch display on
    lcd_write_command(LCD_CMD_DISPLAY_ON);
}

void lcd_send_data(uint8_t *buf, uint32_t len) {
    uint32_t i, x, y;

    // set start to 0,0
    lcd_write_command(LCD_CMD_SET_STARTLINE + 0);
    lcd_write_command(LCD_CMD_SET_PAGESTART + 2);

    // set col address of ram to 0
    lcd_write_command(LCD_CMD_SET_COL_LO + 0);
    lcd_write_command(LCD_CMD_SET_COL_HI + 0);

    for (y = 0; y < 8; y++) {
        // start on col 0
        lcd_write_command(LCD_CMD_SET_COL_LO + 0);
        lcd_write_command(LCD_CMD_SET_COL_HI + 0);
        // page start
        lcd_write_command(LCD_CMD_SET_PAGESTART + y);

        LCD_CS_LO();
        LCD_RS_HI();
        LCD_RW_LO();

        // end 4 dummy bytes(132-128)
        for (x = 4; x > 0; --x) {
            LCD_DATA_SET(0x00);
            LCD_RD_HI();
            LCD_RD_LO();
        }

        for (x = 128; x > 0; --x) {
            LCD_DATA_SET(*buf++);
            // execute write
            LCD_RD_HI();
            LCD_RD_LO();
        }
    }

    LCD_RD_HI();

    // deselect device
    LCD_CS_HI();
    LCD_RW_HI();
}
