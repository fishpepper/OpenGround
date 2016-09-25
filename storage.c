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

#include "storage.h"
#include "debug.h"
#include "wdt.h"
#include "delay.h"
#include "led.h"
#include "frsky.h"
#include ".hoptable.h"

#define FRSKY_USE_FIXED_ID 1
//http://www.st.com/content/ccc/resource/technical/document/application_note/2e/d4/65/6b/87/dd/40/25/DM00049914.pdf/files/DM00049914.pdf/jcr:content/translations/en.DM00049914.pdf
//run time copy of persistant storage data:
STORAGE_DESC storage;

void storage_init(void){
    uint8_t i;

    debug("storage: init\n"); debug_flush();

    //init storage
    storage_init_memory();

    //reload data from flash
    storage_read_from_flash();

    if (storage.version != STORAGE_VERSION_ID){
        debug("storage: corrupted! bad version\n"); debug_flush();
        storage_load_defaults();
    }

    debug("storage: loaded hoptable[]:\n");
    for(i=0; i<9; i++){
            debug_put_hex8(storage.frsky_hop_table[i]);
            debug_putc(' ');
    }
    debug("...\n");
    debug("storage: txid 0x"); debug_put_hex16(storage.frsky_txid);
    debug_flush();

}

static void storage_load_defaults(void) {
    uint8_t i;

    debug("storage: reading defaults\n"); debug_flush();

    static const uint8_t tmp[] = FRSKY_HOPTABLE;

    //set valid version
    storage.version = STORAGE_VERSION_ID;

    //load values from .hoptable.h
    storage.frsky_txid[0] = (FRSYK_TXID>>8) & 0xFF;
    storage.frsky_txid[1] = FRSYK_TXID & 0xFF;

    storage.frsky_freq_offset = FRSKY_DEFAULT_FSCAL_VALUE;

    //copy hoptable
    for(i=0; i<FRSKY_HOPTABLE_SIZE; i++){
        storage.frsky_hop_table[i] = tmp[i];
    }

    //save changes
    storage_write_to_flash();
}

static void storage_init_memory(void){
    //TODO
}

static void storage_read_from_flash(void){
    uint8_t *storage_ptr;
    uint16_t len;

    debug("storage: reading\n"); debug_flush();
    storage_ptr = (uint8_t*)&storage;
    len = sizeof(storage);

    storage_read(storage_ptr, len);
}

void storage_write_to_flash(void){
    uint8_t *storage_ptr ;
    uint16_t len;

    debug("storage: writing\n"); debug_flush();
    storage.version = STORAGE_VERSION_ID;

    storage_ptr = (uint8_t*)&storage;
    len = sizeof(storage);

    //execute flash write:
    storage_write(storage_ptr, len);
}


static void storage_write(uint8_t *buffer, uint16_t len){

}

static void storage_read(uint8_t *storage_ptr, uint16_t len){

}


