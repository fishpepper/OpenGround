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

#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#include <stdint.h>
#include "fifo.h"

void telemetry_init(void);
void telemetry_enqueue(uint8_t byte);
void telemetry_process(void);

static void telemetry_parse_stream(uint8_t byte);
static void telemetry_process_hub_packet(uint8_t id, uint16_t data);

uint16_t telemetry_get_voltage(void);
uint16_t telemetry_get_current(void);
uint16_t telemetry_get_mah(void);

// FrSky telemetry stream state machine
typedef enum {
  TELEMETRY_IDLE = 0,   // waiting for 0x5e frame marker
  TELEMETRY_DATA_ID,    // waiting for dataID
  TELEMETRY_DATA_LOW,   // waiting for data low byte
  TELEMETRY_DATA_HIGH,  // waiting for data high byte
  TELEMETRY_DATA_END,   // waiting for 0x5e end of frame marker
  TELEMETRY_XOR = 0x80  // decode stuffed byte
} telemetry_state_t;


#endif  // TELEMETRY_H_
