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

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

#include "config.h"
#include "delay.h"

void spi_init(void);
void spi_dma_xfer(uint8_t *buffer, uint8_t len);
#define spi_csn_lo() { gpio_clear(CC2500_SPI_GPIO, CC2500_SPI_CSN_PIN); delay_us(1); }
#define spi_csn_hi() { delay_us(1); gpio_set(CC2500_SPI_GPIO, CC2500_SPI_CSN_PIN); }
uint8_t spi_tx(uint8_t data);
uint8_t spi_rx(void);
uint8_t spi_read_address(uint8_t address);

#endif  // SPI_H_

