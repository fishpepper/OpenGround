#ifndef __SPI__H_
#define __SPI__H_
#include <stdint.h>
#include "config.h"
#include "delay.h"

void spi_init(void);
static void spi_init_gpio(void);
static void spi_init_mode(void);
static void spi_init_dma(void);
static void spi_init_rcc(void);
static void spi_enable(void);
void spi_dma_xfer(uint8_t *buffer, uint8_t len);
#define spi_csn_lo() { CC25XX_SPI_GPIO->BRR  = (CC25XX_SPI_CSN_PIN); delay_us(1); }
#define spi_csn_hi() { delay_us(1); CC25XX_SPI_GPIO->BSRR = (CC25XX_SPI_CSN_PIN); }
uint8_t spi_tx(uint8_t address);
uint8_t spi_rx(void);
uint8_t spi_read_address(uint8_t address);

#endif // __SPI__H_

