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

#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdint.h>

#include "frsky.h"

#define STORAGE_VERSION_ID 0x02
#define STORAGE_MODEL_NAME_LEN 11
#define STORAGE_MODEL_MAX_COUNT 10

void storage_init(void);
// static void storage_init_memory(void);
// void storage_write_to_flash(void);
// static void storage_read_from_flash(void);
void storage_save(void);
void storage_load(void);
void storage_mode_set_name(uint8_t index, uint8_t *str);
static void storage_load_defaults(void);
/*static void storage_write(uint8_t *buffer, uint16_t len);
static void storage_read(uint8_t *storage_ptr, uint16_t len);*/

// model description
typedef struct {
    // name of the model
    uint8_t name[STORAGE_MODEL_NAME_LEN];
    // timer
    uint16_t timer;
    // add further data here...
} MODEL_DESC;

// our storage struct contains all data that has to be stored on flash
typedef struct {
    // version id
    uint8_t version;
    // persistent storage for frsky
    uint8_t frsky_txid[2];
    uint8_t frsky_hop_table[FRSKY_HOPTABLE_SIZE];
    int8_t  frsky_freq_offset;
    // stick calibration data
    uint16_t stick_calibration[4][3];
    // model settings
    uint8_t current_model;
    MODEL_DESC model[STORAGE_MODEL_MAX_COUNT];
    // add further data here...
} STORAGE_DESC;

// rounded up
#define SIZEOF_STORAGE_IN_16BIT ((sizeof(storage) + 1) / 2)

extern STORAGE_DESC storage;

#endif  // STORAGE_H_
