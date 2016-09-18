#ifndef __ADC_H__
#define __ADC_H__
#include "main.h"
#include <stdint.h>

void adc_init(void);

static void adc_init_rcc(void);
static void adc_init_gpio(void);
static void adc_init_mode(void);
static void adc_init_dma(void);
static void adc_dma_arm(void);
void adc_process(void);
uint16_t adc_get_channel(uint32_t id);
uint32_t adc_get_battery_voltage(void);

#define ADC_DMA_CHANNEL           DMA1_Channel1
#define ADC_DMA_TC_FLAG           DMA1_FLAG_TC1
#define ADC_CHANNEL_COUNT 11

#endif
