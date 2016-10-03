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

#ifndef ADC_H_
#define ADC_H_
#include <stdint.h>
#include "main.h"

void adc_init(void);
void adc_test(void);

static void adc_init_rcc(void);
static void adc_init_gpio(void);
static void adc_init_mode(void);
static void adc_init_dma(void);
static void adc_dma_arm(void);

void adc_process(void);

uint16_t adc_get_channel(uint32_t id);
int32_t adc_get_channel_rescaled(uint8_t idx);
uint16_t adc_get_channel_packetdata(uint8_t idx);
uint32_t adc_get_battery_voltage(void);

#define ADC_DMA_CHANNEL           DMA1_Channel1
#define ADC_DMA_TC_FLAG           DMA1_FLAG_TC1
#define ADC_CHANNEL_COUNT 11

#define ADC_CHANNEL_AILERON   0
#define ADC_CHANNEL_ELEVATION 1
#define ADC_CHANNEL_THROTTLE  2
#define ADC_CHANNEL_RUDDER    3
#define ADC_CHANNEL_CH0       4
#define ADC_CHANNEL_CH1       5
#define ADC_CHANNEL_CH2       6
#define ADC_CHANNEL_CH3       7

// rescaled data goes from -3200 to 3200
// set zero threshold to 10% movement from absolute zero
#define ADC_RESCALED_ABSOLUTE_MIN        -3200
#define ADC_RESCALED_ABSOLUTE_MAX         3200
#define ADC_RESCALED_ZERO_THRESHOLD      (ADC_RESCALED_ABSOLUTE_MIN + 0.1 * \
                                         (ADC_RESCALED_ABSOLUTE_MAX - ADC_RESCALED_ABSOLUTE_MIN))


#endif  // ADC_H_
