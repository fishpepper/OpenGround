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

#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include <stdint.h>
#include <libopencmsis/core_cm3.h>

void timeout_init(void);
// void timeout_set(__IO uint32_t ms);
#define timeout_set(x) timeout_set_100us(10*(x));
void timeout_set_100us(__IO uint32_t hus);
void timeout2_set_100us(__IO uint32_t hus);
void timeout2_delay_100us(uint16_t us);
uint8_t timeout_timed_out(void);
uint8_t timeout2_timed_out(void);
void timeout_delay_ms(uint32_t timeout);
uint32_t timeout_time_remaining(void);

#endif  // TIMEOUT_H_
