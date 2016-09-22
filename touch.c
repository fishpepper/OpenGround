/*
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

#include "touch.h"
#include "delay.h"
#include "debug.h"
#include "timeout.h"
#include "lcd.h"
#include "io.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_i2c.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_syscfg.h"

#define TOUCH_I2C_DEBUG         0
#define TOUCH_I2C_TIMEOUT      20
#define TOUCH_I2C_FLAG_TIMEOUT 10

static touch_event_t touch_event;

void touch_init(void) {
    debug("touch: init\n"); debug_flush();

    touch_deinit_i2c();
    touch_init_i2c_rcc();

    //free bus with pulse train
    touch_init_i2c_free_bus();

    touch_init_i2c_gpio();
    touch_init_i2c_mode();


    touch_ft6236_init();

    touch_init_isr();
}

static void touch_deinit_i2c(void){
    //disable i2c:
    I2C_Cmd(TOUCH_I2C, DISABLE);
    I2C_DeInit(TOUCH_I2C);
}

static void touch_init_i2c_rcc(void) {
    //I2C CLK source
    RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);

    // gpio clocks
    RCC_AHBPeriphClockCmd(TOUCH_I2C_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(TOUCH_INT_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(TOUCH_RESET_GPIO_CLK, ENABLE);

    // peripheral clock for i2c
    RCC_APB1PeriphClockCmd(TOUCH_I2C_CLK, ENABLE);
}

static void touch_init_i2c_gpio(void) {
    GPIO_InitTypeDef gpio_init;
    GPIO_StructInit(&gpio_init);

    //set up alternate function
    GPIO_PinAFConfig(TOUCH_I2C_GPIO, GPIO_PinSource8, GPIO_AF_1);
    GPIO_PinAFConfig(TOUCH_I2C_GPIO, GPIO_PinSource9, GPIO_AF_1);

    //SCL
    gpio_init.GPIO_Pin   = TOUCH_I2C_SCL_PIN;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_OType = GPIO_OType_OD;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TOUCH_I2C_GPIO, &gpio_init);

    //SDA
    gpio_init.GPIO_Pin   = TOUCH_I2C_SDA_PIN;
    GPIO_Init(TOUCH_I2C_GPIO, &gpio_init);

    //INT pin
    gpio_init.GPIO_Pin  = TOUCH_INT_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_IN;
    gpio_init.GPIO_PuPd = GPIO_PuPd_UP;
    gpio_init.GPIO_OType = GPIO_OType_OD;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TOUCH_INT_GPIO, &gpio_init);

    //RESET pin
    gpio_init.GPIO_Pin  = TOUCH_RESET_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TOUCH_RESET_GPIO, &gpio_init);
}

static void touch_init_isr(void) {
    EXTI_InitTypeDef   exti_init;
    NVIC_InitTypeDef   nvic_init;

    //enable a pin change interrupt on the INT line
    //falling edge
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    //Connect EXTI0 Line to TOUCH controller INT pin
    SYSCFG_EXTILineConfig(TOUCH_INT_EXTI_SOURCE, TOUCH_INT_EXTI_SOURCE_PIN);

    //configure EXTI0 line
    exti_init.EXTI_Line    = TOUCH_INT_EXTI_SOURCE_LINE;
    exti_init.EXTI_Mode    = EXTI_Mode_Interrupt;
    exti_init.EXTI_Trigger = EXTI_Trigger_Falling;
    exti_init.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti_init);


    //enable and set EXTI* Interrupt
    nvic_init.NVIC_IRQChannel = TOUCH_INT_EXTI_IRQN;
    nvic_init.NVIC_IRQChannelPriority = 0x00;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);
}

//void EXTI0_1_IRQHandler(void)
//    if( (EXTI->IMR & EXTI_IMR_MR0) && (EXTI->PR & EXTI_PR_PR0)){
//        EXTI0Flag = 1;
//         delay(50000);
//         while(GPIOA->IDR & GPIO_IDR_0){}
//         EXTI->PR |= EXTI_PR_PR0 ;
//        }
//}

void EXTI4_15_IRQHandler(void) {
    if(EXTI_GetITStatus(TOUCH_INT_EXTI_SOURCE_LINE) != RESET){
        //interrupt (falling edge) on Touch INT line, event detected!
        if (touch_event.event_id == 0){
            //last event was picked up, store new one
            touch_ft6236_packet_t buf;
            uint32_t res = touch_i2c_read(0x00, (uint8_t *)&buf, sizeof(buf));

            if (!res){
                debug("touch: error rd touchdata\n");
                debug_flush();
            }else{
                //fine, touch data arrived, process
                //debug_put_newline(); debug_put_hex8(buf.gest_id);debug_put_newline();
                if (buf.gest_id & TOUCH_FT6236_GESTURE_MOVE_FLAG){
                    //gesture for us! -> overwrite clicks
                    touch_event.event_id = (buf.gest_id & 0x0F) + 1;
                    touch_event.x = 0;
                    touch_event.y = 0;
                }else{
                    //process clicks:
                    uint32_t touch_count = buf.touches & 0xf;
                    if (touch_count > 0) {
                        //always use first touch point
                        uint8_t ev = buf.points[0].event >> 6;
                        touch_event.event_id = TOUCH_GESTURE_MOUSE_DOWN + ev;
                        //swap x&y and calculate lcd pixel coords
                        touch_event.y = (buf.points[0].xhi & 0x0F)<<8 | (buf.points[0].xlo);
                        touch_event.y = (touch_event.y>>1);
                        touch_event.x = (buf.points[0].yhi & 0x0F)<<8 | (buf.points[0].ylo);
                        touch_event.x = 128- - (touch_event.x>>1);
                    }
                }
            }
        }

        //clear the EXTI line pending bit
        EXTI_ClearITPendingBit(TOUCH_INT_EXTI_SOURCE_LINE);
    }
}

static void touch_init_i2c_free_bus(void) {
    uint8_t i;
    GPIO_InitTypeDef gpio_init;
    GPIO_StructInit(&gpio_init);

    debug("touch: freeing i2c bus\n");
    debug_flush();

    //gpio init:
    // reset i2c bus by setting clk as output and sending manual clock pulses
    gpio_init.GPIO_Pin   = TOUCH_I2C_SCL_PIN;
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TOUCH_I2C_GPIO, &gpio_init);

    gpio_init.GPIO_Pin   = TOUCH_I2C_SDA_PIN;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN;
    gpio_init.GPIO_OType = GPIO_OType_OD;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TOUCH_I2C_GPIO, &gpio_init);

    //send 100khz clock train for some 100ms
    timeout_set(100);
    while(!timeout_timed_out()){
        if (GPIO_ReadInputDataBit(TOUCH_I2C_GPIO, TOUCH_I2C_SDA_PIN) == 1){
            debug("touch: i2c free again\n");
            break;
        }
        TOUCH_I2C_GPIO->BSRR = TOUCH_I2C_SCL_PIN;
        delay_us(10);
        TOUCH_I2C_GPIO->BRR  = TOUCH_I2C_SCL_PIN;
        delay_us(10);
    }

    //send stop condition:
    gpio_init.GPIO_Pin   = TOUCH_I2C_SDA_PIN;
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TOUCH_I2C_GPIO, &gpio_init);

    //clock is low
    TOUCH_I2C_GPIO->BRR = TOUCH_I2C_SCL_PIN;
    delay_us(10);
    //sda = lo
    TOUCH_I2C_GPIO->BRR = TOUCH_I2C_SDA_PIN;
    delay_us(10);
    //clock goes high
    TOUCH_I2C_GPIO->BSRR = TOUCH_I2C_SCL_PIN;
    delay_us(10);
    //sda = hi
    TOUCH_I2C_GPIO->BRR = TOUCH_I2C_SDA_PIN;
    delay_us(10);
}

static void touch_init_i2c_mode(void) {
    I2C_InitTypeDef  i2c_init;
    I2C_StructInit(&i2c_init);

    i2c_init.I2C_Mode          = I2C_Mode_I2C;
    i2c_init.I2C_AnalogFilter  = I2C_AnalogFilter_Enable;
    i2c_init.I2C_DigitalFilter = 0x00;
    i2c_init.I2C_OwnAddress1   = 0x00;
    i2c_init.I2C_Ack           = I2C_Ack_Enable;
    i2c_init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    // 400KHz | 8MHz-0x00310309; 16MHz-0x10320309; 48MHz-50330309
    i2c_init.I2C_Timing        = 0x50330309;

    //apply I2C configuration
    I2C_Init(TOUCH_I2C, &i2c_init);

    //enable i2c
    I2C_Cmd(TOUCH_I2C, ENABLE);
}

static void touch_ft6236_reset(void) {
    //do a hw reset:
    TOUCH_RESET_LO();
    delay_ms(20);
    TOUCH_RESET_HI();
    delay_ms(300);
}

static void touch_ft6236_init(void) {
    uint32_t res;
    uint8_t data;

    //do a hw reset
    touch_ft6236_reset();

    //try to detect the device:
    res = touch_i2c_read(0x00, &data, 1);
    if (!res) {
        debug("touch: failed to detect ft6236\n"); debug_flush();
        return;
    }

    //show debug info:
    touch_ft6236_debug_info();
}

static uint8_t touch_i2c_read_byte(uint8_t reg) {
    uint32_t res;
    uint8_t data;

    //read one byte
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

static uint32_t touch_i2c_read(uint8_t address, uint8_t *data, uint8_t len){
    if (TOUCH_I2C_DEBUG){
        debug("touch: i2c read_buffer(0x");
        debug_put_hex8(address);
        debug(", ..., ");
        debug_put_uint8(len);
        debug(")\n");
        debug_flush();
    }

    timeout_set(TOUCH_I2C_TIMEOUT);
    while(I2C_GetFlagStatus(TOUCH_I2C, I2C_ISR_BUSY) != RESET){
        if (timeout_timed_out()) {
            debug("touch: bus busy... timeout!\n");
            return 0;
        }
    }

    //Configure slave address, nbytes, reload, end mode and start or stop generation
    I2C_TransferHandling(TOUCH_I2C, TOUCH_FT6236_I2C_ADDRESS, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);

    //wait for completion
    timeout_set(TOUCH_I2C_FLAG_TIMEOUT);
    while(I2C_GetFlagStatus(TOUCH_I2C, I2C_ISR_TXIS) == RESET){
        if (timeout_timed_out()) {
            debug("touch: start error... timeout!\n");
            debug_flush();
            delay_ms(5000);
            return 0;
        }
    }

    //send the address to read from
    I2C_SendData(TOUCH_I2C, address);

    //wait for completion
    timeout_set(TOUCH_I2C_FLAG_TIMEOUT);
    while(I2C_GetFlagStatus(TOUCH_I2C, I2C_ISR_TC) == RESET){
        if (timeout_timed_out()) {
            debug("touch: send address... timeout!\n");
            return 0;
        }
    }

    // send START condition a second time
    I2C_TransferHandling(TOUCH_I2C, TOUCH_FT6236_I2C_ADDRESS, len, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);

    // wait for completion
    uint16_t i;
    for(i=0; i<len; i++){
        timeout_set(TOUCH_I2C_FLAG_TIMEOUT);
        while(I2C_GetFlagStatus(TOUCH_I2C, I2C_ISR_RXNE) == RESET){
            if (timeout_timed_out()) {
                debug("touch: rx error... timeout!\n");
                return 0;
            }
        }

        //store data
        data[i] = I2C_ReceiveData(TOUCH_I2C);
        //if (TOUCH_I2C_DEBUG) debug("\nrx 0x"); debug_put_hex8(data[i]); debug_flush();

     }

    //wait for stopf set
    timeout_set(TOUCH_I2C_FLAG_TIMEOUT);
    while(I2C_GetFlagStatus(TOUCH_I2C, I2C_ISR_STOPF) == RESET){
        if (timeout_timed_out()) {
            debug("touch: stop error... timeout!\n");
            return 0;
        }
    }

    //clear STOPF flag
    I2C_ClearFlag(TOUCH_I2C, I2C_ICR_STOPCF);

    return 1;
}


touch_event_t touch_get_and_clear_last_event(void){
    touch_event_t tmp = touch_event;
    touch_event.event_id = 0;
    return tmp;
}

void touch_test(void){
    debug("TOUCH TEST\n");
    debug_flush();
    //powerdown in 10seconds
    uint32_t delay = 20;
    uint32_t powerdown_counter = 10*(1000/delay);
    while(powerdown_counter--){
        touch_event_t t = touch_get_and_clear_last_event();
        if (t.event_id){
            //detected touch event!
            uint32_t ev_valid = 1;
            switch(t.event_id){
                default:
                    debug("UNKNOWN 0x");
                    debug_put_hex8(t.event_id);
                    break;

                case(TOUCH_GESTURE_UP):
                    debug("UP!");
                    break;

                case(TOUCH_GESTURE_DOWN):
                    debug("DOWN!");
                    break;

                case(TOUCH_GESTURE_LEFT):
                    debug("LEFT!");
                    break;

                case(TOUCH_GESTURE_RIGHT):
                    debug("RIGHT!");
                    break;

                case(TOUCH_GESTURE_MOUSE_DOWN):
                    debug("CLICK ");
                    debug_put_uint16(t.x);
                    debug_putc(' ');
                    debug_put_uint16(t.y);
                    break;

                case(TOUCH_GESTURE_MOUSE_UP):
                case(TOUCH_GESTURE_MOUSE_MOVE):
                case(TOUCH_GESTURE_MOUSE_NONE):
                    //ignore those for now
                    ev_valid = 0;
                    break;
            }
            if (ev_valid){
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
