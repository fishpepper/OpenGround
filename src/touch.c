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

#include "touch.h"
#include "delay.h"
#include "debug.h"
#include "timeout.h"
#include "lcd.h"
#include "io.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/stm32/common/i2c_common_v2.h>


// internal functions
static void touch_deinit_i2c(void);
static void touch_init_i2c_mode(void);
static void touch_init_i2c_gpio(void);
static void touch_init_i2c_rcc(void);
static void touch_init_i2c_free_bus(void);

static uint32_t touch_i2c_read(uint8_t address, uint8_t *data, uint8_t len);
static uint8_t touch_i2c_read_byte(uint8_t reg);
static void touch_ft6236_debug_info(void);
static void touch_init_isr(void);
static void touch_ft6236_init(void);


#define TOUCH_I2C_DEBUG         1
#define TOUCH_I2C_TIMEOUT      20
#define TOUCH_I2C_FLAG_TIMEOUT 10

static volatile touch_event_t touch_event;

void touch_init(void) {
    debug("touch: init\n"); debug_flush();

    touch_event.event_id = 0;

    touch_deinit_i2c();
    touch_init_i2c_rcc();

    // free bus with pulse train
    if (0) touch_init_i2c_free_bus();

    touch_init_i2c_gpio();
    touch_init_i2c_mode();


    touch_ft6236_init();

    touch_init_isr();
}

static void touch_deinit_i2c(void) {
    // disable i2c:
    i2c_peripheral_disable(TOUCH_I2C);
    i2c_reset(TOUCH_I2C);
}


static void touch_init_i2c_rcc(void) {
    // I2C CLK source
    rcc_periph_clock_enable(TOUCH_I2C_CLK);

    // gpio clocks
    rcc_periph_clock_enable(GPIO_RCC(TOUCH_I2C_GPIO));
    rcc_periph_clock_enable(GPIO_RCC(TOUCH_INT_GPIO));
    rcc_periph_clock_enable(GPIO_RCC(TOUCH_RESET_GPIO));
}

static void touch_init_i2c_gpio(void) {
    // set up alternate function
    gpio_set_af(TOUCH_I2C_GPIO, GPIO_AF1, TOUCH_I2C_SCL_PIN);
    gpio_set_af(TOUCH_I2C_GPIO, GPIO_AF1, TOUCH_I2C_SDA_PIN);

    // SCL
    gpio_mode_setup(TOUCH_I2C_GPIO, GPIO_MODE_AF, GPIO_PUPD_PULLUP, TOUCH_I2C_SCL_PIN);
    gpio_set_output_options(TOUCH_I2C_GPIO, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, TOUCH_I2C_SCL_PIN);

    // SDA
    gpio_mode_setup(TOUCH_I2C_GPIO, GPIO_MODE_AF, GPIO_PUPD_PULLUP, TOUCH_I2C_SDA_PIN);
    gpio_set_output_options(TOUCH_I2C_GPIO, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, TOUCH_I2C_SDA_PIN);

    // INT pin
    gpio_mode_setup(TOUCH_INT_GPIO, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, TOUCH_INT_PIN);

    // RESET pin
    gpio_mode_setup(TOUCH_RESET_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, TOUCH_RESET_PIN);
}

static void touch_init_isr(void) {
    // enable a pin change interrupt on the INT line (falling edge)

    // clock for syscfg
    rcc_periph_clock_enable(RCC_SYSCFG_COMP);

    // connect EXTI12 Line to TOUCH controller INT pin
    exti_select_source(TOUCH_INT_EXTI_SOURCE_LINE, TOUCH_INT_EXTI_SOURCE);
    exti_set_trigger(TOUCH_INT_EXTI_SOURCE_LINE, EXTI_TRIGGER_FALLING);
    exti_enable_request(TOUCH_INT_EXTI_SOURCE_LINE);

    // enable irq
    nvic_enable_irq(TOUCH_INT_EXTI_IRQN);
    nvic_set_priority(TOUCH_INT_EXTI_IRQN, NVIC_PRIO_TOUCH);
}

void EXTI4_15_IRQHandler(void) {
    if (exti_get_flag_status(TOUCH_INT_EXTI_SOURCE_LINE) != 0) {
        exti_reset_request(TOUCH_INT_EXTI_SOURCE_LINE);

        // interrupt(falling edge) on Touch INT line, event detected!
        if (touch_event.event_id == 0) {
            // last event has been processed
            touch_ft6236_packet_t buf;

            // fetch data:
            uint32_t res = touch_i2c_read(0x00, (uint8_t *)&buf, sizeof(buf));

            if (!res) {
                debug("touch: error rd touchdata\n");
                debug_flush();
            } else {
                // fine, touch data arrived, process
                // debug_put_newline(); debug_put_hex8(buf.gest_id);debug_put_newline();
                if (buf.gest_id & TOUCH_FT6236_GESTURE_MOVE_FLAG) {
                    // gesture for us! -> overwrite clicks
                    touch_event.event_id = (buf.gest_id & 0x0F) + 1;
                    touch_event.x = 0;
                    touch_event.y = 0;
                } else {
                    // process clicks:
                    uint32_t touch_count = buf.touches & 0xf;
                    if (touch_count > 0) {
                        // always use first touch point
                        uint8_t ev = buf.points[0].event >> 6;
                        touch_event.event_id = TOUCH_GESTURE_MOUSE_DOWN + ev;
                        // swap x&y and calculate lcd pixel coords
                        touch_event.y = (buf.points[0].xhi & 0x0F) << 8  | (buf.points[0].xlo);
                        touch_event.y = (touch_event.y >> 1);
                        touch_event.x = (buf.points[0].yhi & 0x0F) << 8 | (buf.points[0].ylo);
                        touch_event.x = 128 - (touch_event.x >> 1);

                        // correct for strange offset bug in screen center
                        // on my touch screen the touch points in the center
                        // are off... no idea why
                        /* if (touch_event.x > (LCD_WIDTH / 2 - 10) &&
                            (touch_event.x < (LCD_WIDTH / 2 + 10))) {
                            // substract a value of 10 at center, linear to the sides
                            if (touch_event.x > LCD_WIDTH / 2) {
                                touch_event.y -= 10 - (touch_event.x - (LCD_WIDTH / 2));
                            } else {
                                touch_event.y -= 10 - ((LCD_WIDTH / 2) - touch_event.x);
                            }
                        }
                        debug_put_uint16(touch_event.x);
                        debug_putc(' ');
                        debug_put_uint16(touch_event.y);
                        debug_put_newline();
                        debug_flush();*/
                    }
                }
            }
        }

        // clear the EXTI line pending bit
        // EXTI_ClearITPendingBit(TOUCH_INT_EXTI_SOURCE_LINE);
    }
}


touch_event_t touch_get_last_event(void) {
    touch_event_t tmp = touch_event;
    touch_event.event_id = 0;
    return tmp;
}

static void touch_init_i2c_free_bus(void) {
    debug("touch: freeing i2c bus\n");
    debug_flush();

    // gpio init:
    // reset i2c bus by setting clk as output and sending manual clock pulses
    gpio_mode_setup(TOUCH_I2C_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TOUCH_I2C_SCL_PIN);
    gpio_set_output_options(TOUCH_I2C_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, TOUCH_I2C_SCL_PIN);

    gpio_mode_setup(TOUCH_I2C_GPIO, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, TOUCH_I2C_SDA_PIN);
    gpio_set_output_options(TOUCH_I2C_GPIO, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, TOUCH_I2C_SDA_PIN);

    // send 100khz clock train for some 100ms
    timeout_set(100);
    while (!timeout_timed_out()) {
        if (gpio_get(TOUCH_I2C_GPIO, TOUCH_I2C_SDA_PIN) == 1) {
            debug("touch: i2c free again\n");
            break;
        }
        gpio_set(TOUCH_I2C_GPIO, TOUCH_I2C_SCL_PIN);
        delay_us(10);
        gpio_clear(TOUCH_I2C_GPIO, TOUCH_I2C_SCL_PIN);
        delay_us(10);
    }

    // send stop condition:
    gpio_mode_setup(TOUCH_I2C_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, TOUCH_I2C_SDA_PIN);
    gpio_set_output_options(TOUCH_I2C_GPIO, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, TOUCH_I2C_SDA_PIN);

    // clock is low
    gpio_clear(TOUCH_I2C_GPIO, TOUCH_I2C_SCL_PIN);
    delay_us(10);
    // sda = lo
    gpio_clear(TOUCH_I2C_GPIO, TOUCH_I2C_SDA_PIN);
    delay_us(10);
    // clock goes high
    gpio_set(TOUCH_I2C_GPIO, TOUCH_I2C_SCL_PIN);
    delay_us(10);
    // sda = hi
    gpio_clear(TOUCH_I2C_GPIO, TOUCH_I2C_SDA_PIN);
    delay_us(10);
}

static void touch_init_i2c_mode(void) {
    // disable during init
    i2c_peripheral_disable(TOUCH_I2C);

    i2c_enable_analog_filter(TOUCH_I2C);
    i2c_set_digital_filter(TOUCH_I2C, 0x00);
    i2c_set_own_7bit_slave_address(TOUCH_I2C, 0x00);
    i2c_set_7bit_addr_mode(TOUCH_I2C);

    // 400KHz | 8MHz-0x00310309; 16MHz-0x10320309; 48MHz-50330309
    I2C_TIMINGR(TOUCH_I2C) = 0x50330309;

    i2c_peripheral_enable(TOUCH_I2C);

    // ACK ENABLE? set?? CR2 &= ~(I2C_CR2_NACK)
}

static void touch_ft6236_reset(void) {
    // do a hw reset:
    TOUCH_RESET_LO();
    delay_ms(20);
    TOUCH_RESET_HI();
    delay_ms(300);
}

static void touch_ft6236_init(void) {
    uint32_t res;
    uint8_t data;

    // do a hw reset
    touch_ft6236_reset();

    // try to detect the device:
    res = touch_i2c_read(0x00, &data, 1);
    if (!res) {
        debug("touch: failed to detect ft6236\n"); debug_flush();
        delay_ms(5000);
        return;
    }

    // show debug info:
    touch_ft6236_debug_info();
}

static uint8_t touch_i2c_read_byte(uint8_t reg) {
    uint32_t res;
    uint8_t data;

    // read one byte
    res = touch_i2c_read(reg, &data, 1);
    if (!res) {
        debug("touch: failed to read from ft6236\n"); debug_flush();
        return 0;
    }
    return data;
}


static void touch_ft6236_debug_info(void) {
    debug("touch: thrhld =");
    debug_put_uint8(touch_i2c_read_byte(TOUCH_FT6236_REG_TH_GROUP) * 4);
    debug_put_newline();

    debug("touch: rep rate=");
    debug_put_uint8(touch_i2c_read_byte(TOUCH_FT6236_REG_PERIODACTIVE) * 10);
    debug_put_newline();

    debug("touch: fw lib 0x");
    debug_put_hex8(touch_i2c_read_byte(TOUCH_FT6236_REG_LIB_VER_H));
    debug_put_hex8(touch_i2c_read_byte(TOUCH_FT6236_REG_LIB_VER_L));
    debug_put_newline();

    debug("touch: fw v 0x");
    debug_put_hex8(touch_i2c_read_byte(TOUCH_FT6236_REG_FIRMID));
    debug_put_newline();

    debug("touch: CHIP ID 0x");
    debug_put_hex8(touch_i2c_read_byte(TOUCH_FT6236_REG_CIPHER));
    debug_put_newline();

    debug("touch: CTPM ID 0x");
    debug_put_hex8(touch_i2c_read_byte(TOUCH_FT6236_REG_FOCALTECH_ID));
    debug_put_newline();

    debug("touch: rel code 0x");
    debug_put_hex8(touch_i2c_read_byte(TOUCH_FT6236_REG_RELEASE_CODE_ID));
    debug_put_newline();
}

static uint32_t touch_i2c_read(uint8_t reg, uint8_t *data, uint8_t len) {
    int wait;
    int i;

    if (TOUCH_I2C_DEBUG) {
        debug("touch: i2c read_buffer(0x");
        debug_put_hex8(reg);
        debug(", ..., ");
        debug_put_uint8(len);
        debug(")\n");
        debug_flush();
    }


    // wait for not busy
    timeout_set(TOUCH_I2C_TIMEOUT);
    while (i2c_busy(TOUCH_I2C) == 1) {
        if (timeout_timed_out()) {
            debug("touch: bus busy... timeout!\n");
            return 0;
        }
    }

    // wait for not start busy
    timeout_set(TOUCH_I2C_TIMEOUT);
    while (i2c_is_start(TOUCH_I2C) == 1) {
        if (timeout_timed_out()) {
            debug("touch: is start timeout!\n");
            return 0;
        }
    }

    // setting transfer properties
    i2c_set_bytes_to_transfer(TOUCH_I2C, 1);
    i2c_set_7bit_address(TOUCH_I2C, TOUCH_FT6236_I2C_ADDRESS);
    i2c_set_write_transfer_dir(TOUCH_I2C);
    i2c_disable_autoend(TOUCH_I2C);

    // start transfer
    i2c_send_start(TOUCH_I2C);

    wait = true;

    // wait for tx finish
    timeout_set(TOUCH_I2C_TIMEOUT);
    while (wait) {
        if (i2c_transmit_int_status(TOUCH_I2C)) {
                wait = false;
        }
        if (timeout_timed_out()) {
            debug("touch: start timeout!\n");
            return 0;
        }
    }

    // nack?
    timeout_set(TOUCH_I2C_TIMEOUT);
    while (i2c_nack(TOUCH_I2C)) {
        if (timeout_timed_out()) {
            debug("touch: nack timeout!\n");
            return 0;
        }
    }

    i2c_send_data(TOUCH_I2C, reg);

    timeout_set(TOUCH_I2C_TIMEOUT);
    while (i2c_is_start(TOUCH_I2C) == 1) {
        if (timeout_timed_out()) {
            debug("touch: is start timeout!\n");
            return 0;
        }
    }

    // Setting transfer properties
    i2c_set_bytes_to_transfer(TOUCH_I2C, len);
    i2c_set_7bit_address(TOUCH_I2C, TOUCH_FT6236_I2C_ADDRESS);
    i2c_set_read_transfer_dir(TOUCH_I2C);
    i2c_enable_autoend(TOUCH_I2C);

    // start transfer
    i2c_send_start(TOUCH_I2C);

    for (i = 0; i < len; i++) {
          // rx timeout
          timeout_set(TOUCH_I2C_TIMEOUT);
          while (i2c_received_data(TOUCH_I2C) == 0) {
              if (timeout_timed_out()) {
                  debug("touch: rx timeout!\n");
                  return 0;
              }
          }

          data[i] = i2c_get_data(TOUCH_I2C);
    }

    return 1;
}



void touch_test(void) {
    debug("TOUCH TEST\n");
    debug_flush();
    // powerdown in 10seconds
    uint32_t delay = 20;
    uint32_t powerdown_counter = 10*(1000/ delay);
    while (powerdown_counter--) {
        touch_event_t t = touch_get_last_event();
        if (t.event_id) {
            // detected touch event!
            uint32_t ev_valid = 1;
            switch (t.event_id) {
                default:
                    debug("UNKNOWN 0x");
                    debug_put_hex8(t.event_id);
                    break;

                case (TOUCH_GESTURE_UP):
                    debug("UP!");
                    break;

                case (TOUCH_GESTURE_DOWN):
                    debug("DOWN!");
                    break;

                case (TOUCH_GESTURE_LEFT):
                    debug("LEFT!");
                    break;

                case (TOUCH_GESTURE_RIGHT):
                    debug("RIGHT!");
                    break;

                case (TOUCH_GESTURE_MOUSE_DOWN):
                    debug("CLICK ");
                    debug_put_uint16(t.x);
                    debug_putc(' ');
                    debug_put_uint16(t.y);
                    break;

                case (TOUCH_GESTURE_MOUSE_UP):
                case (TOUCH_GESTURE_MOUSE_MOVE):
                case (TOUCH_GESTURE_MOUSE_NONE):
                    // ignore those for now
                    ev_valid = 0;
                    break;
            }
            if (ev_valid) {
                debug_put_newline();
                debug_flush();
            }
        }
        delay_ms(delay);
    }
    debug("will power down now\n"); debug_flush();

    lcd_powerdown();
    io_powerdown();
}
