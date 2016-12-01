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

#include "crc16.h"

// lookup table for crc16 CCITT
static const uint16_t crc16_table[16] = {
  0x0000, 0x1081, 0x2102, 0x3183, 0x4204, 0x5285, 0x6306, 0x7387,
  0x8408, 0x9489, 0xA50A, 0xB58B, 0xC60C, 0xD68D, 0xE70E, 0xF78F
};

static uint16_t crc16_update(uint16_t crc, uint8_t c) {
    crc = (((crc >> 4) & 0x0FFF) ^ crc16_table[((crc ^ c) & 0x000F)]);
    crc = (((crc >> 4) & 0x0FFF) ^ crc16_table[((crc ^ (c>>4)) & 0x000F)]);
    return crc;
}

uint16_t crc16(uint8_t *buf, uint16_t len) {
    uint16_t crc = 0;
    while (len--) {
        crc = crc16_update(crc, *buf++);
    }
    return crc;
}
