EXTERN(vector_table)
ENTRY(reset_handler)

/* reserve two pages for eeprom emulation */
_emulated_eeprom_page_size = 2048; /* do not change this, stm32f072 has 2k pagesize */
_emulated_eeprom_size = 2*_emulated_eeprom_page_size;

MEMORY
{
 ram (rwx) : ORIGIN = 0x20000000, LENGTH = 16K
 rom (rx) : ORIGIN = 0x08000000, LENGTH = 128K
 EMULATED_EEPROM (rwx) : ORIGIN = 0x8000000+128K-2*2048 LENGTH=2*2048
}


/*EMULATED_EEPROM;*/
_emulated_eeprom = 0x08011800; /*ORIGIN(EMULATED_EEPROM);*/


SECTIONS
{
 .text : {
  *(.vectors)
  *(.text*)
  . = ALIGN(4);
  *(.rodata*)
  . = ALIGN(4);
 } >rom
 .preinit_array : {
  . = ALIGN(4);
  __preinit_array_start = .;
  KEEP (*(.preinit_array))
  __preinit_array_end = .;
 } >rom
 .init_array : {
  . = ALIGN(4);
  __init_array_start = .;
  KEEP (*(SORT(.init_array.*)))
  KEEP (*(.init_array))
  __init_array_end = .;
 } >rom
 .fini_array : {
  . = ALIGN(4);
  __fini_array_start = .;
  KEEP (*(.fini_array))
  KEEP (*(SORT(.fini_array.*)))
  __fini_array_end = .;
 } >rom
 .ARM.extab : {
  *(.ARM.extab*)
 } >rom
 .ARM.exidx : {
  __exidx_start = .;
  *(.ARM.exidx*)
  __exidx_end = .;
 } >rom
 . = ALIGN(4);
 _etext = .;
 .data : {
  _data = .;
  *(.data*)
  . = ALIGN(4);
  _edata = .;
 } >ram AT >rom
 _data_loadaddr = LOADADDR(.data);
 .bss : {
  *(.bss*)
  *(COMMON)
  . = ALIGN(4);
  _ebss = .;
 } >ram
 /DISCARD/ : { *(.eh_frame) }
 . = ALIGN(4);
 end = .;
}
PROVIDE(_stack = ORIGIN(ram) + LENGTH(ram));
