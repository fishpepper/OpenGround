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


#include "eeprom.h"
#include "debug.h"
#include "delay.h"
#include "storage.h"

#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/flash.h>

// init for st eeprom emulation, set up number of variables.
uint16_t EE_virtual_address_table[EE_NB_OF_VAR];

#include "eeprom_emulation/st_eeprom.h"

void eeprom_init(void) {
    debug("eeprom: init\n"); debug_flush();

    // initialize virtual adresses
    uint32_t i;
    for (i = 0; i < EE_NB_OF_VAR; i++) {
        EE_virtual_address_table[i] = i;
    }

    // unlock flash for writing
    flash_unlock();

    // initialise eeprom emulation
    uint16_t res = EE_Init();

    if (res != FLASH_COMPLETE) {
        debug("eeprom: EE_init failed: 0x");
        debug_put_hex16(res);
        debug_put_newline();
        debug_flush();
    }

    // lock flash again
    flash_lock();
}

void eeprom_write_storage(void) {
    debug("eeprom: writing storage\n"); debug_flush();
    uint8_t *storage_ptr;
    uint16_t p = 0;
    uint16_t i;

    storage_ptr = (uint8_t*)&storage;

    /*debug("eeprom: storing "); debug_put_uint16(sizeof(storage));
    debug("b\n");
    debug("in "); debug_put_uint16(EE_NB_OF_VAR);
    debug(" 16b entries.");*/
    flash_unlock();
    for (i = 0; i < EE_NB_OF_VAR; i++) {
        // fetch current value. make sure that out of bound access to storage reads as zero
        uint16_t value_now = 0;
        if (p < sizeof (storage)) value_now  = storage_ptr[p++] << 8;
        if (p < sizeof (storage)) value_now |= storage_ptr[p++];

        /*debug("storage: wr 0x");
        if ((p-2) < sizeof(storage)) { debug_put_hex8(storage_ptr[p-2]); } else { debug("--");}
        debug(" 0x");
        if ((p-1) < sizeof(storage)) { debug_put_hex8(storage_ptr[p-1]); } else { debug("--");}
        debug("\n-> combined to 0x");
        debug_put_hex16(value_now);
        debug_put_newline();
        debug_flush();
        if (i == 0) delay_ms(5000);*/

        // fetch value from eeprom in order to check for a change
        uint16_t value_ee;
        uint16_t res = EE_ReadVariable(EE_virtual_address_table[i], &value_ee);

        /*if (res != 0) {
            debug("eeprom: fetch failed for 0x");
            debug_put_hex16(i);
            debug_put_newline();
            debug_flush();
        }*/

        if (value_now != value_ee) {
            debug("eeprom: val["); debug_put_hex16(i); debug("] modified\n");
            res = EE_WriteVariable(EE_virtual_address_table[i], value_now);
            if (res != FLASH_COMPLETE) {
                debug("eeprom: write failed for 0x");
                debug_put_hex16(i);
                debug_put_newline();
                debug_flush();
            }
        }
    }
    flash_lock();
    // uint32_t t; for (t = 0; t < 20; t++) { delay_ms(100); wdt_reset(); }
}


void eeprom_read_storage(void) {
    debug("eeprom: reading storage\n"); debug_flush();
    uint8_t *storage_ptr;
    uint16_t p = 0;
    uint16_t i;

    // pointer to persistant storage
    storage_ptr = (uint8_t*)&storage;

    // invalidate storage
    storage.version = 0;

    flash_unlock();
    for (i = 0; i < EE_NB_OF_VAR; i++) {
        // fetch current 16bit value
        uint16_t value;

        // fetch value from eeprom in order to check for a change
        uint16_t res = EE_ReadVariable(EE_virtual_address_table[i], &value);

        if (res != 0) {
            /*debug("eeprom: fetch failed for 0x");
            debug_put_hex16(i);
            debug_put_newline();
            debug_flush();*/
        } else {
            /*debug("eeprom: read 0x"); debug_put_hex16(value); debug_put_newline(); debug_flush();
            delay_ms(100);*/

            // read back into storage:
            if (p < sizeof (storage)) storage_ptr[p++] = (value>>8) & 0xFF;
            if (p < sizeof (storage)) storage_ptr[p++] = value & 0xFF;

            /*if (i == 0) {
                debug("eeprom: 0x"); debug_put_hex8(storage_ptr[0]);
                debug(" 0x"); debug_put_hex8(storage_ptr[1]); debug_put_newline();
                debug("s.v = 0x"); debug_put_hex8(storage.version);
                debug_put_newline(); debug_flush();
                delay_ms(5000);
            }*/
        }
    }
    // debug_put_hex16(storage.checksum);
    flash_lock();
    // uint32_t t; for (t = 0; t < 20; t++) { delay_ms(100); wdt_reset(); }
}


