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


#include "spi.h"
#include "debug.h"
#include "led.h"
#include "config.h"
#include "cc2500.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>

// internal functions
static void spi_init_gpio(void);
static void spi_init_mode(void);
static void spi_init_dma(void);
static void spi_init_rcc(void);


void spi_init(void) {
    debug("spi: init\n"); debug_flush();
    spi_init_rcc();
    spi_init_gpio();
    spi_init_mode();
    spi_init_dma();
    /*while(1) {
        delay_ms(10);
        // select device:
        cc2500_csn_lo();
        spi_tx(0xAB);
        // select device:
        cc2500_csn_hi();

    }*/
}

static void spi_init_rcc(void) {
    // enable clocks
    rcc_periph_clock_enable(GPIO_RCC(CC2500_SPI_GPIO));
    rcc_periph_clock_enable(CC2500_SPI_CLK);
}

static void spi_init_mode(void) {
    // SPI NVIC
    // nvic_set_priority(NVIC_SPI2_IRQ, 3);
    // nvic_enable_irq(NVIC_SPI2_IRQ);

    // clean start
    spi_reset(CC2500_SPI);

    // set up spi
    // - master mode
    // - baud prescaler = apb_clk/8 = 24/8 = 3MHz!
    // - CPOL low
    // - CPHA 1
    // - 8 bit crc (?)
    // - MSB first
    spi_init_master(CC2500_SPI,
                    SPI_CR1_BAUDRATE_FPCLK_DIV_8,
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1,
                    SPI_CR1_CRCL_8BIT,
                    SPI_CR1_MSBFIRST);

    // set NSS to software
    // NOTE: setting NSS high is important! even when controling it on our
    //       own. otherwise spi will not send any data!
    spi_enable_software_slave_management(CC2500_SPI);
    spi_set_nss_high(CC2500_SPI);

    // Enable SPI periph
    spi_enable(CC2500_SPI);



    // set fifo to quarter full(=1 byte)
    spi_fifo_reception_threshold_8bit(CC2500_SPI);
}



static void spi_init_dma(void) {
    debug("spi: init dma\n"); debug_flush();

    // enable DMA1 Peripheral Clock
    rcc_periph_clock_enable(RCC_DMA);

    // DMA NVIC
    nvic_set_priority(NVIC_DMA1_CHANNEL2_3_IRQ, NVIC_PRIO_FRSKY);
    // nvic_enable_irq(NVIC_DMA1_CHANNEL2_3_IRQ); // NO IRQ beeing used here

    // start with clean init for RX channel
    dma_channel_reset(DMA1, CC2500_SPI_RX_DMA_CHANNEL);
    // source and destination 1Byte = 8bit
    dma_set_memory_size(DMA1, CC2500_SPI_RX_DMA_CHANNEL, DMA_CCR_MSIZE_8BIT);
    dma_set_peripheral_size(DMA1, CC2500_SPI_RX_DMA_CHANNEL, DMA_CCR_PSIZE_8BIT);
    // automatic memory destination increment enable.
    dma_enable_memory_increment_mode(DMA1, CC2500_SPI_RX_DMA_CHANNEL);
    // source address increment disable
    dma_disable_peripheral_increment_mode(DMA1, CC2500_SPI_RX_DMA_CHANNEL);
    // Location assigned to peripheral register will be source
    dma_set_read_from_peripheral(DMA1, CC2500_SPI_RX_DMA_CHANNEL);
    // source and destination start addresses
    dma_set_peripheral_address(DMA1, CC2500_SPI_RX_DMA_CHANNEL, (uint32_t)&CC2500_SPI_DR);
    // target address will be set later
    dma_set_memory_address(DMA1, CC2500_SPI_RX_DMA_CHANNEL, 0);
    // chunk of data to be transfered, will be set later
    dma_set_number_of_data(DMA1, CC2500_SPI_RX_DMA_CHANNEL, 1);
    // very high prio
    dma_set_priority(DMA1, CC2500_SPI_RX_DMA_CHANNEL, DMA_CCR_PL_VERY_HIGH);

    // start with clean init for TX channel
    dma_channel_reset(DMA1, CC2500_SPI_TX_DMA_CHANNEL);
    // source and destination 1Byte = 8bit
    dma_set_memory_size(DMA1, CC2500_SPI_TX_DMA_CHANNEL, DMA_CCR_MSIZE_8BIT);
    dma_set_peripheral_size(DMA1, CC2500_SPI_TX_DMA_CHANNEL, DMA_CCR_PSIZE_8BIT);
    // automatic memory destination increment enable.
    dma_enable_memory_increment_mode(DMA1, CC2500_SPI_TX_DMA_CHANNEL);
    // source address increment disable
    dma_disable_peripheral_increment_mode(DMA1, CC2500_SPI_TX_DMA_CHANNEL);
    // Location assigned to peripheral register will be target
    dma_set_read_from_memory(DMA1, CC2500_SPI_TX_DMA_CHANNEL);
    // source and destination start addresses
    dma_set_peripheral_address(DMA1, CC2500_SPI_TX_DMA_CHANNEL, (uint32_t)&CC2500_SPI_DR);
    // target address will be set later
    dma_set_memory_address(DMA1, CC2500_SPI_TX_DMA_CHANNEL, 0);
    // chunk of data to be transfered, will be set later
    dma_set_number_of_data(DMA1, CC2500_SPI_TX_DMA_CHANNEL, 1);
    // very high prio
    dma_set_priority(DMA1, CC2500_SPI_TX_DMA_CHANNEL, DMA_CCR_PL_VERY_HIGH);

    // start disabled
    dma_disable_channel(DMA1, CC2500_SPI_RX_DMA_CHANNEL);
    dma_disable_channel(DMA1, CC2500_SPI_TX_DMA_CHANNEL);
}

// data in buffer will be sent and will be overwritten with
// the data read back from the spi slave
void spi_dma_xfer(uint8_t *buffer, uint8_t len) {
    // debug("xfer "); debug_put_uint8(len); debug(")\n");

    // TX: transfer buffer to slave
    dma_set_memory_address(DMA1, CC2500_SPI_TX_DMA_CHANNEL, (uint32_t)buffer);
    dma_set_number_of_data(DMA1, CC2500_SPI_TX_DMA_CHANNEL, len);

    // RX: read back data from slave
    dma_set_memory_address(DMA1, CC2500_SPI_RX_DMA_CHANNEL, (uint32_t)buffer);
    dma_set_number_of_data(DMA1, CC2500_SPI_RX_DMA_CHANNEL, len);

    // enable both dma channels
    dma_enable_channel(DMA1, CC2500_SPI_RX_DMA_CHANNEL);
    dma_enable_channel(DMA1, CC2500_SPI_TX_DMA_CHANNEL);

    // trigger the SPI TX + RX dma
    spi_enable_tx_dma(CC2500_SPI);
    spi_enable_rx_dma(CC2500_SPI);

#if 0
    // Wait until the command is sent to the DR
    while (!DMA_GetFlagStatus(CC2500_SPI_TX_DMA_TC_FLAG)) {}

    // debug("ACTIVE\n"); debug_flush();

    // wait for tx to be finished:
    while (DMA_GetFlagStatus(CC2500_SPI_TX_DMA_TC_FLAG)) {}
    while (DMA_GetFlagStatus(CC2500_SPI_RX_DMA_TC_FLAG)) {}

    // wait for SPI to be no longer busy
    while (SPI_I2S_GetFlagStatus(CC2500_SPI, SPI_I2S_FLAG_BSY) != RESET) {}
    // debug("!BUSY\n"); debug_flush();
#endif  // 0

    // wait for completion
    while (!(SPI_SR(CC2500_SPI) & SPI_SR_TXE)) {}
    while (SPI_SR(CC2500_SPI) & SPI_SR_BSY) {}

    // disable DMA
    dma_disable_channel(DMA1, CC2500_SPI_RX_DMA_CHANNEL);
    dma_disable_channel(DMA1, CC2500_SPI_TX_DMA_CHANNEL);
}


static void spi_init_gpio(void) {
    // init sck, mosi and miso
    uint32_t spi_gpios = CC2500_SPI_SCK_PIN | CC2500_SPI_MOSI_PIN | CC2500_SPI_MISO_PIN;
    // set mode
    gpio_mode_setup(CC2500_SPI_GPIO, GPIO_MODE_AF, GPIO_PUPD_NONE, spi_gpios);
    gpio_set_output_options(CC2500_SPI_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, spi_gpios);
    // set pin to alternate function
    gpio_set_af(CC2500_SPI_GPIO, GPIO_AF1, spi_gpios);

    // configure csn
    gpio_mode_setup(CC2500_SPI_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CC2500_SPI_CSN_PIN);
    gpio_set_output_options(CC2500_SPI_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CC2500_SPI_CSN_PIN);

    // start deselected
    spi_csn_hi();
}

uint8_t spi_tx(uint8_t data) {
    spi_send8(CC2500_SPI, data);
    return spi_read8(CC2500_SPI);
}


uint8_t spi_rx(void) {
    spi_send8(CC2500_SPI, 0xFF);
    return spi_read8(CC2500_SPI);
}
