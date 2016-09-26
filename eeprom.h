#ifndef __EEPROM_H__
#define __EEPROM_H__
#include <stdint.h>
#include "storage.h"

void eeprom_init(void);
void eeprom_write_storage(void);

//init for st eeprom emulation, set up number of variables.
#define EE_NB_OF_VAR             ((uint16_t)(SIZEOF_STORAGE_IN_16BIT))
uint16_t EE_virtual_address_table[EE_NB_OF_VAR];

#endif // __EEPROM_H__
