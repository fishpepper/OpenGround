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
#include "screen.h"
#include "console.h"
#include "led.h"
#include "wdt.h"
#include "delay.h"
#include "storage.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dma.h>

static uint16_t adc_data[ADC_CHANNEL_COUNT];
static uint16_t adc_battery_voltage_raw_filtered;

void adc_init(void) {
    debug("adc: init\n"); debug_flush();

    adc_battery_voltage_raw_filtered = 0;

    adc_init_rcc();
    adc_init_gpio();
    adc_init_mode();
    adc_init_dma();

    // init values(for debugging)
    uint32_t i;
    for (i = 0; i < ADC_CHANNEL_COUNT; i++) {
        adc_data[i] = i;
    }
}

uint16_t adc_get_channel(uint32_t id) {
    // fetch correct adc channel based on hw revision
    if (config_hw_revision == CONFIG_HW_REVISION_I6S) {
        // FS-i6S mapping:
        switch (id) {
            case (CHANNEL_ID_AILERON)  : return adc_data[0];
            case (CHANNEL_ID_ELEVATION): return adc_data[1];
            case (CHANNEL_ID_THROTTLE) : return adc_data[2];
            case (CHANNEL_ID_RUDDER)   : return adc_data[3];
            case (CHANNEL_ID_CH0)      : return adc_data[4];
            case (CHANNEL_ID_CH1)      : return adc_data[5];
            case (CHANNEL_ID_CH2)      : return adc_data[8];
            case (CHANNEL_ID_CH3)      : return adc_data[9];
        }
    } else if (config_hw_revision == CONFIG_HW_REVISION_EVOLUTION) {
        // TGY Evolution mapping:
        switch (id) {
            case (CHANNEL_ID_AILERON)  : return 4095 - adc_data[3];
            case (CHANNEL_ID_ELEVATION): return 4095 - adc_data[2];
            case (CHANNEL_ID_THROTTLE) : return 4095 - adc_data[1];
            case (CHANNEL_ID_RUDDER)   : return 4095 - adc_data[0];
            case (CHANNEL_ID_CH0)      : return adc_data[5];
            case (CHANNEL_ID_CH1)      : return adc_data[8];
            case (CHANNEL_ID_CH2)      : return adc_data[6];
            case (CHANNEL_ID_CH3)      : return adc_data[4];
        }
    } else {
        // undefined!
        debug("adc: invalid hw revision ");
        debug_put_uint8(config_hw_revision);
        debug(" given!\n"); debug_flush();
        return 0;
    }
}

char *adc_get_channel_name(uint8_t i, bool short_descr) {
    switch (i) {
        default                     : return ((short_descr) ? "?" : "???");
        case (CHANNEL_ID_AILERON)   : return ((short_descr) ? "A" : "AIL");
        case (CHANNEL_ID_ELEVATION) : return ((short_descr) ? "E" : "ELE");
        case (CHANNEL_ID_THROTTLE)  : return ((short_descr) ? "T" : "THR");
        case (CHANNEL_ID_RUDDER)    : return ((short_descr) ? "R" : "RUD");
        case (CHANNEL_ID_CH0)       : return ((short_descr) ? "0" : "CH0");
        case (CHANNEL_ID_CH1)       : return ((short_descr) ? "1" : "CH1");
        case (CHANNEL_ID_CH2)       : return ((short_descr) ? "2" : "CH2");
        case (CHANNEL_ID_CH3)       : return ((short_descr) ? "3" : "CH3");
    }
}


#define ADC_RESCALE_TARGET_RANGE 3200
// return the adc channel rescaled from 0...4095 to -TARGET_RANGE...+TARGET_RANGE
// switches are scaled manually, sticks use calibration data
int32_t adc_get_channel_rescaled(uint8_t idx) {
    int32_t divider;

    // fetch raw stick value (0..4095)
    int32_t value = adc_get_channel(idx);

    // sticks are ch0..3 and use calibration coefficents:
    if (idx < 4) {
        // apply center calibration value:
        value = value - (int16_t)storage.stick_calibration[idx][1];

        // now rescale this to +/- TARGET_RANGE
        // fetch divider
        if (value < 0) {
            divider = storage.stick_calibration[idx][1] - storage.stick_calibration[idx][0];
        } else {
            divider = storage.stick_calibration[idx][2] - storage.stick_calibration[idx][1];
        }
        // apply the scale
        value = (value * ADC_RESCALE_TARGET_RANGE) / divider;
    } else {
        // for sticks we do not care about scaling/calibration (for now)
        // min is 0, max from adc is 4095 -> rescale this to +/- 3200
        // rescale to 0...6400
        value = (ADC_RESCALE_TARGET_RANGE * 2 * value) / 4096;
        value = value - ADC_RESCALE_TARGET_RANGE;
    }

    // apply the scale factor:
    switch (idx) {
        default:
            // do not apply scale
            break;
        case (CHANNEL_ID_AILERON):
        case (CHANNEL_ID_ELEVATION):
            value = (value * storage.model[storage.current_model].stick_scale) / 100;
            break;
    }

    // limit value to -3200 ... 3200
    value = max(-ADC_RESCALE_TARGET_RANGE, min(ADC_RESCALE_TARGET_RANGE, value));

    return value;
}

uint16_t adc_get_channel_packetdata(uint8_t idx) {
    // frsky packets send us * 1.5
    // where 1000 us =   0%
    //       2000 us = 100%
    // -> remap +/-3200 to 1500..3000
    // 6400 => 1500 <=> 64 = 15
    int32_t val = adc_get_channel_rescaled(idx);
    val = (15 * val) / 64;
    val = val + 2250;
    return (uint16_t) val;
}


static void adc_init_rcc(void) {
    debug("adc: init rcc\n"); debug_flush();

    // enable adc clock
    rcc_periph_clock_enable(RCC_ADC);

    // enable adc gpio clock
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);

    // start with adc off
    adc_power_off(ADC1);

    // ADC CLOCK = 48 / 4 = 12MHz
    adc_set_clk_source(ADC1, ADC_CLKSOURCE_PCLK_DIV4);

    // run calibration
    adc_calibrate(ADC1);

    // enable dma clock
    rcc_periph_clock_enable(RCC_DMA);
}

static void adc_init_gpio(void) {
    debug("adc: init gpio\n"); debug_flush();

    // set up analog inputs ADC0...ADC7(PA0...PA7)
    gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, 0b11111111);

    // set up analog inputs ADC8, ADC9(PB0, PB1)
    gpio_mode_setup(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, 0b00000011);

    // battery voltage is on PC0(ADC10)
    gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, 0b00000001);
}

uint32_t adc_get_battery_voltage(void) {
    // return a fixed point number of the battery voltage
    // 1230 = 12.3 V
    // raw data is 0 .. 4095 ~ 0 .. 3300mV
    // Vadc = raw * 3300 / 4095
    uint32_t raw = adc_battery_voltage_raw_filtered;
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

    // set mode to continous
    adc_set_continuous_conversion_mode(ADC1);

    // no ext trigger
    adc_disable_external_trigger_regular(ADC1);
    // right 12-bit data alignment in ADC reg
    adc_set_right_aligned(ADC1);
    adc_set_resolution(ADC1, ADC_RESOLUTION_12BIT);

    // adc_enable_temperature_sensor();
    adc_disable_analog_watchdog(ADC1);

    // configure channels 0...10
    uint8_t channels[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // sample times for all channels
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPTIME_041DOT5);

    adc_set_regular_sequence(ADC1, sizeof(channels), channels);

    adc_power_on(ADC1);

    // wait for ADC starting up
    int i;
    for (i = 0; i < 800000; i++) {
        asm("nop");
    }



    // enable DMA for ADC
    adc_enable_dma(ADC1);
}


static void adc_init_dma(void) {
    debug("adc: init dma\n"); debug_flush();

    // clean init
    dma_channel_reset(DMA1, ADC_DMA_CHANNEL);

    // DO NOT use circular mode, we will retrigger dma on our own!
    dma_enable_circular_mode(DMA1, ADC_DMA_CHANNEL);


    // high priority
    dma_set_priority(DMA1, ADC_DMA_CHANNEL, DMA_CCR_PL_HIGH);

    // source and destination 16bit
    dma_set_memory_size(DMA1, ADC_DMA_CHANNEL, DMA_CCR_MSIZE_16BIT);
    dma_set_peripheral_size(DMA1, ADC_DMA_CHANNEL, DMA_CCR_PSIZE_16BIT);

    // automatic memory destination increment enable.
    dma_enable_memory_increment_mode(DMA1, ADC_DMA_CHANNEL);

    // source address increment disable
    dma_disable_peripheral_increment_mode(DMA1, ADC_DMA_CHANNEL);

    // Location assigned to peripheral register will be source
    dma_set_read_from_peripheral(DMA1, ADC_DMA_CHANNEL);

    // source and destination start addresses
    dma_set_peripheral_address(DMA1, ADC_DMA_CHANNEL, (uint32_t)&ADC1_DR);
    dma_set_memory_address(DMA1, ADC_DMA_CHANNEL, (uint32_t)adc_data);

    // chunk of data to be transfered
    dma_set_number_of_data(DMA1, ADC_DMA_CHANNEL, ADC_CHANNEL_COUNT);

    // start conversion:
    adc_dma_arm();
}

static void adc_dma_arm(void) {
    // start conversion
    dma_enable_channel(DMA1, ADC_DMA_CHANNEL);
    adc_start_conversion_regular(ADC1);
}

void adc_process(void) {
    // adc dma finished?
    if (dma_get_interrupt_flag(DMA1, ADC_DMA_CHANNEL, ADC_DMA_TC_FLAG)) {
        dma_clear_interrupt_flags(DMA1, ADC_DMA_CHANNEL, ADC_DMA_TC_FLAG);
        if (adc_battery_voltage_raw_filtered == 0) {
            // initialise with current value
            adc_battery_voltage_raw_filtered = adc_data[10];
        } else {
            // low pass filter battery voltage
            adc_battery_voltage_raw_filtered = adc_battery_voltage_raw_filtered +
                                     (4 * (adc_data[10] - adc_battery_voltage_raw_filtered)) / 128;
        }

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
        screen_update();
        led_button_r_toggle();

        // delay_us(10*1000);
        wdt_reset();
        delay_ms(100);
    }
}

