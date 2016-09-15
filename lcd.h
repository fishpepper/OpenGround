#ifndef __LCD__H_
#define __LCD__H_
#include <stdint.h>
#include "stm32f0xx_gpio.h"
#include "config.h"

void lcd_init(void);
static void lcd_init_gpio(void);
static void lcd_init_rcc(void);
static void lcd_reset(void);
static void lcd_write_command(uint8_t data);
void lcd_send_data(uint8_t *buf, uint32_t len);


#define LCD_RW_HI()   { LCD_RW_GPIO->BSRR = (LCD_RW_PIN); delay_us(1); }
#define LCD_RW_LO()   { LCD_RW_GPIO->BRR  = (LCD_RW_PIN); delay_us(1);}

#define LCD_RST_HI()  { LCD_RST_GPIO->BSRR = (LCD_RST_PIN); delay_us(1);}
#define LCD_RST_LO()  { LCD_RST_GPIO->BRR  = (LCD_RST_PIN); delay_us(1);}

#define LCD_RS_HI()   { LCD_RS_GPIO->BSRR = (LCD_RS_PIN); delay_us(1);}
#define LCD_RS_LO()   { LCD_RS_GPIO->BRR  = (LCD_RS_PIN); delay_us(1);}

#define LCD_RD_HI()   { LCD_RD_GPIO->BSRR = (LCD_RD_PIN); delay_us(1);}
#define LCD_RD_LO()   { LCD_RD_GPIO->BRR  = (LCD_RD_PIN); delay_us(1);}

#define LCD_CS_HI()   { LCD_CS_GPIO->BSRR = (LCD_CS_PIN); delay_us(1);}
#define LCD_CS_LO()   { LCD_CS_GPIO->BRR  = (LCD_CS_PIN); delay_us(1);}

#define LCD_DATA_SET(data) { ((uint8_t __IO*)&LCD_DATA_GPIO->ODR)[0] = (data); delay_us(1);}



#define LCD_CMD_RESET            0xE2
#define LCD_CMD_BIAS_1_9         0xA2 // 1/9 bias
#define LCD_CMD_BIAS_1_7         0xA3 // 1/7 bias

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


// the screen itself is 128 x 64, however the display ram of the controler
// has 132x64 entries. therefore we define the framebuffer to have the same size
// in order to be able to push this to the display without any calculations
#define LCD_WIDTH   128
#define LCD_HEIGHT   64


#endif //__LCD__H_

