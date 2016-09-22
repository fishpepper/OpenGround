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

#include "spi.h"
#include "debug.h"
#include "led.h"
#include "config.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_spi.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_dma.h"


void spi_init(void) {
    spi_init_rcc();
    spi_init_gpio();
    spi_init_mode();
    spi_init_dma();
    spi_enable();
}

static void spi_init_rcc(void) {
    // enable clocks
    RCC_AHBPeriphClockCmd(CC2500_SPI_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(CC2500_SPI_CLK, ENABLE);
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
    spi_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //3mhz
    spi_init.SPI_FirstBit  = SPI_FirstBit_MSB;
    spi_init.SPI_CRCPolynomial = 7;
    SPI_Init(CC2500_SPI, &spi_init);
}



static void spi_init_dma(void) {
    DMA_InitTypeDef dma_init;

    // Enable DMA1 Peripheral Clock
    RCC_AHBPeriphClockCmd(CC2500_SPI_DMA_CLOCK, ENABLE);

    // Configure SPI RX Channel
    dma_init.DMA_DIR                = DMA_DIR_PeripheralSRC;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&CC2500_SPI->DR;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryBaseAddr     = 0; // will be set later
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_init.DMA_BufferSize         = 1; // will be set later
    dma_init.DMA_Mode               = DMA_Mode_Normal;
    dma_init.DMA_Priority           = DMA_Priority_VeryHigh;
    dma_init.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(CC2500_SPI_RX_DMA_CHANNEL, &dma_init);

    // configure SPI TX Channel
    dma_init.DMA_DIR                = DMA_DIR_PeripheralDST;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)&CC2500_SPI->DR;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryBaseAddr     = 0; // will be set later
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_init.DMA_BufferSize         = 1; // will be set later
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
    //debug("xfer "); debug_put_uint8(len); debug(")\n");

    // TX: transfer buffer to slave
    CC2500_SPI_TX_DMA_CHANNEL->CMAR  = (uint32_t)buffer;
    CC2500_SPI_TX_DMA_CHANNEL->CNDTR = len;

    // RX: read back data from slave
    CC2500_SPI_RX_DMA_CHANNEL->CMAR  = (uint32_t)buffer;
    CC2500_SPI_RX_DMA_CHANNEL->CNDTR = len;

    // enable both dma
    DMA_Cmd(CC2500_SPI_RX_DMA_CHANNEL, ENABLE);
    DMA_Cmd(CC2500_SPI_TX_DMA_CHANNEL, ENABLE);

    //debug("DMA EN\n"); debug_flush();

    // trigger the SPI TX + RX dma
    SPI_I2S_DMACmd(CC2500_SPI, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

    //debug("TRIG\n"); debug_flush();
#if 0
    // Wait until the command is sent to the DR
    while (!DMA_GetFlagStatus(CC2500_SPI_TX_DMA_TC_FLAG)) {};

    //debug("ACTIVE\n"); debug_flush();

    // wait for tx to be finished:
    while (DMA_GetFlagStatus(CC2500_SPI_TX_DMA_TC_FLAG)) {};
    while (DMA_GetFlagStatus(CC2500_SPI_RX_DMA_TC_FLAG)) {};

    //wait for SPI to be no longer busy
    while (SPI_I2S_GetFlagStatus(CC2500_SPI, SPI_I2S_FLAG_BSY) != RESET){}
    //debug("!BUSY\n"); debug_flush();
#endif

    while (SPI_I2S_GetFlagStatus(CC2500_SPI, SPI_I2S_FLAG_TXE) == RESET);
    while (SPI_I2S_GetFlagStatus(CC2500_SPI, SPI_I2S_FLAG_BSY) != RESET){}

    //while((SPI1->SR & 2) == 0);  // wait while TXE flag is 0 (TX is not empty)
    //while((SPI1->SR & (1 << 7)) != 0);  // wait while BSY flag is 1 (SPI is busy)

    //disable DMA
    DMA_Cmd(CC2500_SPI_RX_DMA_CHANNEL, DISABLE);
    DMA_Cmd(CC2500_SPI_TX_DMA_CHANNEL, DISABLE);

    // clear DMA flags
    SPI_I2S_DMACmd(CC2500_SPI,SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);
}


static void spi_init_gpio(void) {
    GPIO_InitTypeDef gpio_init;
    GPIO_StructInit(&gpio_init);

    //set up alternate function
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
    gpio_init.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(CC2500_SPI_GPIO, &gpio_init);

    // configure CSN as Push-Pull
    gpio_init.GPIO_Pin   = CC2500_SPI_CSN_PIN;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_Init(CC2500_SPI_GPIO, &gpio_init);

}

uint8_t spi_tx(uint8_t address){
    // wait for SPI Tx buffer empty
    while (SPI_I2S_GetFlagStatus(CC2500_SPI, SPI_I2S_FLAG_TXE) == RESET);
    // send SPI data
    SPI_SendData8(CC2500_SPI, address);

    // read response
    while (SPI_I2S_GetFlagStatus(CC2500_SPI, SPI_I2S_FLAG_RXNE) == RESET);
    uint8_t result = SPI_ReceiveData8(CC2500_SPI);
    return result;
}


uint8_t spi_rx(void) {
    return spi_tx(0xff);
}
