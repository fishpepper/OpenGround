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

#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>
#include "storage.h"

void eeprom_init(void);
void eeprom_write_storage(void);
void eeprom_read_storage(void);

// init for st eeprom emulation, set up number of variables.
#define EE_NB_OF_VAR             ((uint16_t)(SIZEOF_STORAGE_IN_16BIT))
// extern uint16_t EE_virtual_address_table[EE_NB_OF_VAR];

#endif  // EEPROM_H_
