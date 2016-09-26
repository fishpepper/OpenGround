#ifndef __STORAGE_H__
#define __STORAGE_H__
#include <stdint.h>
#include "frsky.h"

#define STORAGE_VERSION_ID 0x02

void storage_init(void);
//static void storage_init_memory(void);
//void storage_write_to_flash(void);
//static void storage_read_from_flash(void);
void storage_save(void);
static void storage_load(void);

static void storage_load_defaults(void);
/*static void storage_write(uint8_t *buffer, uint16_t len);
static void storage_read(uint8_t *storage_ptr, uint16_t len);*/


//our storage struct contains all data that has to be stored on flash
typedef struct {
    //version id
    uint8_t version;
    //persistent storage for frsky
    uint8_t frsky_txid[2];
    uint8_t frsky_hop_table[FRSKY_HOPTABLE_SIZE];
    int8_t  frsky_freq_offset;
    //add further data here...
    //make sure to add ids to STORAGE_DESC_ID
    //and add a write handler to storage_store() !
} STORAGE_DESC;

//round up:
#define SIZEOF_STORAGE_IN_16BIT ((sizeof(storage)+1) / 2)

extern STORAGE_DESC storage;


#endif
