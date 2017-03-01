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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>


void spi_init(void) {
    debug("spi: init\n"); debug_flush();
    spi_init_rcc();
    spi_init_gpio();
    spi_init_mode();
    spi_init_dma();
    spi_enable();
}

static void spi_init_rcc(void) {
    // enable clocks
    rcc_periph_clock_enable(GPIO_RCC(CC2500_SPI_GPIO));

    rcc_periph_clock_enable(CC2500_SPI_CLK);
    rcc_periph_clock_enable(RCC_DMA);
    rcc_periph_clock_enable(CC2500_SPI);
}

static void spi_enable(void) {
    SPI_Cmd(CC2500_SPI, ENABLE);
}

static void spi_init_mode(void) {
    SPI_InitTypeDef spi_init;

    // mode config
    spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_init.SPI_Mode      = SPI_Mode_Master;
    spi_init.SPI_DataSize  = SPI_DataSize_8b;
    spi_init.SPI_CPOL      = SPI_CPOL_Low;
    spi_init.SPI_CPHA      = SPI_CPHA_1Edge;
    spi_init.SPI_NSS       = SPI_NSS_Soft;
    spi_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;  // 3mHz(!)
    spi_init.SPI_FirstBit  = SPI_FirstBit_MSB;
    spi_init.SPI_CRCPolynomial = 7;
    SPI_Init(CC2500_SPI, &spi_init);

    // set fifo to quarter full(=1 byte)
    SPI_RxFIFOThresholdConfig(CC2500_SPI, SPI_RxFIFOThreshold_QF);
}





/* DMA NVIC */
nvic_set_priority(NVIC_DMA1_CHANNEL4_5_IRQ, 3);
nvic_enable_irq(NVIC_DMA1_CHANNEL4_5_IRQ);

/* SPI NVIC */
nvic_set_priority(NVIC_SPI2_IRQ, 3);
nvic_enable_irq(NVIC_SPI2_IRQ);

/* INIT DMA SPI RX (DMA CHAN4) */
DMA1_IFCR = DMA_IFCR_CGIF4;
DMA1_CCR4 = DMA_CCR_MINC | DMA_CCR_TEIE | DMA_CCR_TCIE;
DMA1_CNDTR4 = 4;
DMA1_CPAR4 = (uint32_t)&SPI2_DR;
DMA1_CMAR4 = (uint32_t)arr_rx;

/* INIT DMA SPI TX (DMA CHAN5) */
DMA1_IFCR = DMA_IFCR_CGIF5;
DMA1_CCR5 = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TEIE | DMA_CCR_TCIE;
DMA1_CNDTR5 = 4;
DMA1_CPAR5 = (uint32_t)&SPI2_DR;
DMA1_CMAR5 = (uint32_t)arr_tx;

/* INIT SPI */
SPI2_I2SCFGR = 0;
SPI2_CR1 = SPI_CR1_BAUDRATE_FPCLK_DIV_256 | SPI_CR1_MSTR | SPI_CR1_BIDIMODE | SPI_CR1_SSM | SPI_CR1_SSI;
SPI2_CR2 = SPI_CR2_DS_8BIT | SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN | SPI_CR2_ERRIE | SPI_CR2_FRXTH;

gpio_clear(GPIOB, GPIO12);

DMA1_CCR4 |= DMA_CCR_EN; /* RX CHAN */
SPI2_CR1 |= SPI_CR1_SPE;
DMA1_CCR5 |= DMA_CCR_EN; /* TX CHAN */

/* LOOP */
for(;;) {
    __asm__("wfi");
}



static void spi_init_dma(void) {
    DMA_InitTypeDef dma_init;

    // enable DMA1 Peripheral Clock
    RCC_AHBPeriphClockCmd(CC2500_SPI_DMA_CLOCK, ENABLE);

    // configure SPI RX Channel
    dma_init.DMA_DIR                = DMA_DIR_PeripheralSRC;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&CC2500_SPI->DR;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryBaseAddr     = 0;  // will be set later
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_init.DMA_BufferSize         = 1;  // will be set later
    dma_init.DMA_Mode               = DMA_Mode_Normal;
    dma_init.DMA_Priority           = DMA_Priority_VeryHigh;
    dma_init.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(CC2500_SPI_RX_DMA_CHANNEL, &dma_init);

    // configure SPI TX Channel
    dma_init.DMA_DIR                = DMA_DIR_PeripheralDST;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&CC2500_SPI->DR;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryBaseAddr     = 0;  // will be set later
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_init.DMA_BufferSize         = 1;  // will be set later
    dma_init.DMA_Mode               = DMA_Mode_Normal;
    dma_init.DMA_Priority           = DMA_Priority_VeryHigh;
    dma_init.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(CC2500_SPI_TX_DMA_CHANNEL, &dma_init);

    // start disabled
    DMA_Cmd(CC2500_SPI_TX_DMA_CHANNEL, DISABLE);
    DMA_Cmd(CC2500_SPI_RX_DMA_CHANNEL, DISABLE);
}

// data in buffer will be sent and will be overwritten with
// the data read back from the spi slave
void spi_dma_xfer(uint8_t *buffer, uint8_t len) {
    // debug("xfer "); debug_put_uint8(len); debug(")\n");

    // TX: transfer buffer to slave
    CC2500_SPI_TX_DMA_CHANNEL->CMAR  = (uint32_t)buffer;
    CC2500_SPI_TX_DMA_CHANNEL->CNDTR = len;

    // RX: read back data from slave
    CC2500_SPI_RX_DMA_CHANNEL->CMAR  = (uint32_t)buffer;
    CC2500_SPI_RX_DMA_CHANNEL->CNDTR = len;

    // enable both dma
    DMA_Cmd(CC2500_SPI_RX_DMA_CHANNEL, ENABLE);
    DMA_Cmd(CC2500_SPI_TX_DMA_CHANNEL, ENABLE);

    // debug("DMA EN\n"); debug_flush();

    // trigger the SPI TX + RX dma
    SPI_I2S_DMACmd(CC2500_SPI, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

    // debug("TRIG\n"); debug_flush();
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

    while (SPI_I2S_GetFlagStatus(CC2500_SPI, SPI_I2S_FLAG_TXE) == RESET) {}
    while (SPI_I2S_GetFlagStatus(CC2500_SPI, SPI_I2S_FLAG_BSY) != RESET) {}

    // while ((SPI1->SR & 2) == 0);  // wait while TXE flag is 0(TX is not empty)
    // while ((SPI1->SR & (1 << 7)) != 0);  // wait while BSY flag is 1(SPI is busy)

    // disable DMA
    DMA_Cmd(CC2500_SPI_RX_DMA_CHANNEL, DISABLE);
    DMA_Cmd(CC2500_SPI_TX_DMA_CHANNEL, DISABLE);

    // clear DMA flags
    SPI_I2S_DMACmd(CC2500_SPI, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);
}


static void spi_init_gpio(void) {

    /* INIT SPI GPIO */
    gpio_mode_setup(CC2500_SPI_GPIO, GPIO_MODE_AF, GPIO_PUPD_NONE, CC2500_SPI_SCK_PINSOURCE | CC2500_SPI_MOSI_PINSOURCE | CC2500_SPI_MISO_PINSOURCE);
    gpio_set_output_options(CC2500_SPI_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, CC2500_SPI_SCK_PINSOURCE | CC2500_SPI_MOSI_PINSOURCE | CC2500_SPI_MISO_PINSOURCE);
    gpio_set_af(GPIOB, GPIO_AF0, GPIO13|GPIO14|GPIO15);

    /* INIT SPI SS GPIO */
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, GPIO12);
    gpio_set(GPIOB, GPIO12);



    GPIO_InitTypeDef gpio_init;
    GPIO_StructInit(&gpio_init);

    // set up alternate function
    GPIO_PinAFConfig(CC2500_SPI_GPIO, CC2500_SPI_SCK_PINSOURCE,  GPIO_AF_1);
    GPIO_PinAFConfig(CC2500_SPI_GPIO, CC2500_SPI_MOSI_PINSOURCE, GPIO_AF_1);
    GPIO_PinAFConfig(CC2500_SPI_GPIO, CC2500_SPI_MISO_PINSOURCE, GPIO_AF_1);

    // configure SCK and MOSI pins as Alternate Function Push-Pull
    gpio_init.GPIO_Pin   = CC2500_SPI_SCK_PIN | CC2500_SPI_MOSI_PIN;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(CC2500_SPI_GPIO, &gpio_init);

    // configure MISO pin as Input floating
    gpio_init.GPIO_Pin  = CC2500_SPI_MISO_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(CC2500_SPI_GPIO, &gpio_init);

    // configure CSN as Push-Pull
    gpio_init.GPIO_Pin   = CC2500_SPI_CSN_PIN;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_Init(CC2500_SPI_GPIO, &gpio_init);
}

uint8_t spi_tx(uint8_t address) {
    // debug("spi: tx 0x"); debug_put_hex8(address); debug_put_newline(); debug_flush();
    // wait for SPI Tx buffer empty
    while (SPI_I2S_GetFlagStatus(CC2500_SPI, SPI_I2S_FLAG_TXE) == RESET) {}
    // send SPI data
    SPI_SendData8(CC2500_SPI, address);

    // read response
    while (SPI_I2S_GetFlagStatus(CC2500_SPI, SPI_I2S_FLAG_RXNE) != SET) {}
    uint8_t result = SPI_ReceiveData8(CC2500_SPI);
    // debug("spi: rx 0x"); debug_put_hex8(result); debug_put_newline(); debug_flush();
    return result;
}


uint8_t spi_rx(void) {
    return spi_tx(0xff);
}
