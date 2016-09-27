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

#include "adc.h"
#include "debug.h"
#include "console.h"
#include "led.h"
#include "wdt.h"
#include "delay.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_dma.h"

static uint16_t adc_data[ADC_CHANNEL_COUNT];

void adc_init(void) {
    debug("adc: init\n"); debug_flush();

    adc_init_rcc();
    adc_init_gpio();
    adc_init_mode();
    adc_init_dma();

    // init values(for debugging)
    uint32_t i;
    for (i = 0; i < ADC_CHANNEL_COUNT; i++) {
        adc_data[i] = 0;
    }
}

uint16_t adc_get_channel(uint32_t id) {
    if (id < ADC_CHANNEL_COUNT) {
        return adc_data[id];
    } else {
        return 0;
    }
}


static void adc_init_rcc(void) {
    debug("adc: init rcc\n"); debug_flush();

    // ADC CLOCK = 24 / 4 = 6MHz
    RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div2);

    // enable ADC clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // enable dma clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // periph clock enable for port
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);
}

static void adc_init_gpio(void) {
    debug("adc: init gpio\n"); debug_flush();

    GPIO_InitTypeDef gpio_init;
    GPIO_StructInit(&gpio_init);

    // set up analog inputs ADC0...ADC7(PA0...PA7)
    gpio_init.GPIO_Pin  = 0b11111111;
    gpio_init.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOA, &gpio_init);

    // set up analog inputs ADC8, ADC9(PB0, PB1)
    gpio_init.GPIO_Pin  = 0b11;
    gpio_init.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOB, &gpio_init);


    // battery voltage is on PC0(ADC10)
    gpio_init.GPIO_Pin  = 0b1;
    gpio_init.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOC, &gpio_init);
}

uint32_t adc_get_battery_voltage(void) {
    // return a fixed point number of the battery voltage
    // 123 = 12.3 V
    // raw data is 0 .. 4095 ~ 0 .. 3300mV
    // Vadc = raw * 3300 / 4095
    uint32_t raw = adc_data[10];
    // the voltage divider is 5.1k / 10k
    // Vadc = Vbat * R2 / (R1+R2) = Vbat * 51/ 151
    // -> Vbat = Vadc * (R1-R2) / R2
    // -> Vout = raw * 3300 * (151 / 51) / 4095
    //         = (raw * (3300 * 151) ) / (4095 * 51)
    uint32_t mv = (raw * (3300 * 151) ) / (4095 * 51);
    return mv / 10;
}

static void adc_init_mode(void) {
    debug("adc: init mode\n"); debug_flush();

    ADC_InitTypeDef adc_init;
    ADC_StructInit(&adc_init);

    // ADC configuration
    adc_init.ADC_ContinuousConvMode   = ENABLE;  // ! select continuous conversion mode
    adc_init.ADC_ExternalTrigConv     = 0;
    adc_init.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;  // select no ext triggering
    adc_init.ADC_DataAlign            = ADC_DataAlign_Right;  // r 12-bit data alignment in ADC reg
    adc_init.ADC_Resolution           = ADC_Resolution_12b;
    adc_init.ADC_ScanDirection        = ADC_ScanDirection_Upward;

    // load structure values to control and status registers
    ADC_Init(ADC1, &adc_init);

    // configure each channel
    ADC_ChannelConfig(ADC1, ADC_Channel_0, ADC_SampleTime_41_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_1, ADC_SampleTime_41_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_2, ADC_SampleTime_41_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_3, ADC_SampleTime_41_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_4, ADC_SampleTime_41_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_5, ADC_SampleTime_41_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_6, ADC_SampleTime_41_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_7, ADC_SampleTime_41_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_8, ADC_SampleTime_41_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_9, ADC_SampleTime_41_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_10, ADC_SampleTime_41_5Cycles);

    // enable ADC
    ADC_Cmd(ADC1, ENABLE);

    // enable DMA for ADC
    ADC_DMACmd(ADC1, ENABLE);
}

static void adc_init_dma(void) {
    debug("adc: init dma\n"); debug_flush();

    DMA_InitTypeDef  dma_init;
    DMA_StructInit(&dma_init);

    // reset DMA1 channe1 to default values
    DMA_DeInit(ADC_DMA_CHANNEL);

    // set up dma to convert 2 adc channels to two mem locations:
    // channel will be used for memory to memory transfer
    dma_init.DMA_M2M                 = DMA_M2M_Disable;
    // setting normal mode(non circular)
    dma_init.DMA_Mode                = DMA_Mode_Circular;
    // medium priority
    dma_init.DMA_Priority            = DMA_Priority_High;
    // source and destination 16bit
    dma_init.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;
    dma_init.DMA_MemoryDataSize      = DMA_MemoryDataSize_HalfWord;
    // automatic memory destination increment enable.
    dma_init.DMA_MemoryInc           = DMA_MemoryInc_Enable;
    // source address increment disable
    dma_init.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
    // Location assigned to peripheral register will be source
    dma_init.DMA_DIR                 = DMA_DIR_PeripheralSRC;
    // chunk of data to be transfered
    dma_init.DMA_BufferSize          = ADC_CHANNEL_COUNT;
    // source and destination start addresses
    dma_init.DMA_PeripheralBaseAddr  = (uint32_t)&ADC1->DR;
    dma_init.DMA_MemoryBaseAddr      = (uint32_t)adc_data;
    // send values to DMA registers
    DMA_Init(ADC_DMA_CHANNEL, &dma_init);

    // enable the DMA1 - Channel1
    DMA_Cmd(ADC_DMA_CHANNEL, ENABLE);

    // start conversion:
    adc_dma_arm();
}

static void adc_dma_arm(void) {
    ADC_StartOfConversion(ADC1);
}

void adc_process(void) {
    // adc dma finished?
    if (DMA_GetITStatus(ADC_DMA_TC_FLAG)) {
        // fine, arm DMA again:
        adc_dma_arm();
    } else {
        // oops this should not happen
        debug_putc('D');
        // cancel and re arm dma ???
    }
}



void adc_test(void) {
    while (1) {
        console_clear();
        debug("ADC TEST  BAT: ");
        debug_put_fixed2(adc_get_battery_voltage());
        debug(" V\n");
        uint32_t i;
        adc_process();
        for (i = 0; i < ADC_CHANNEL_COUNT; i++) {
            debug_put_uint8(i+0); debug_putc('=');
            debug_put_hex16(adc_get_channel(i+0));
            if (i&1) {
                debug_put_newline();
            } else {
                debug(" ");
            }
        }
        console_render();
        led_button_r_toggle();

        // delay_us(10*1000);
        wdt_reset();
        delay_ms(100);
    }
}

