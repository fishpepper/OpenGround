ROOT         := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
SOURCE_DIR   = $(ROOT)/src
INCLUDE_DIR  = $(SOURCE_DIR)
SOURCE_FILES_FOUND = $(wildcard $(SOURCE_DIR)/*.c)
SOURCE_FILES = $(SOURCE_FILES_FOUND:./src/%=%)
SOURCE_FILES += eeprom_emulation/st_eeprom.c
OBJECT_DIR   := $(ROOT)/obj
BIN_DIR      = $(ROOT)/bin
CFLAGS  = -O1 -g
ASFLAGS = -g

# dfu util path
DFU_UTIL ?= dfu-util

# opencm3 lib config
LIBNAME         = opencm3_stm32f0
DEFS            += -DSTM32F0

FP_FLAGS        ?= -msoft-float
ARCH_FLAGS      = -mthumb -mcpu=cortex-m0 $(FP_FLAGS)

LDSCRIPT = linker/stm32f072.ld
TARGET   = openground

CFLAGS += -I./src
LDFLAGS += -L./src

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q		:= @
NULL		:= 2>/dev/null
endif

###############################################################################
# Executables

PREFIX		?= arm-none-eabi

CC		:= $(PREFIX)-gcc
CXX		:= $(PREFIX)-g++
LD		:= $(PREFIX)-gcc
AR		:= $(PREFIX)-ar
AS		:= $(PREFIX)-as
OBJCOPY		:= $(PREFIX)-objcopy
OBJDUMP		:= $(PREFIX)-objdump
GDB		:= $(PREFIX)-gdb
STFLASH		= $(shell which st-flash)
STYLECHECK	:= /checkpatch.pl
STYLECHECKFLAGS	:= --no-tree -f --terse --mailback
STYLECHECKFILES	:= $(shell find . -name '*.[ch]')
OPT		:= -Os
CSTD		?= -std=gnu99


###############################################################################
# objects
OBJS            += $(SOURCE_FILES:%.c=$(OBJECT_DIR)/%.o)
# headers
HEADER_FILES    = $(SOURCE_FILES_FOUND:%.c=%.h)

# where we search for the library
LIBPATHS := ./libopencm3 ../../../../libopencm3 ../../../../../libopencm3
OPENCM3_DIR := ./libopencm3

ifeq ($(V),1)
$(info Using $(OPENCM3_DIR) path to library)
endif

# Old style, assume LDSCRIPT exists
DEFS		+= -I$(OPENCM3_DIR)/include
LDFLAGS		+= -L$(OPENCM3_DIR)/lib
LDLIBS		+= -l$(LIBNAME)

SCRIPT_DIR	= $(OPENCM3_DIR)/scripts

###############################################################################
# C flags

TGT_CFLAGS	+= $(OPT) $(CSTD) -g
TGT_CFLAGS	+= $(ARCH_FLAGS)
TGT_CFLAGS	+= -Wextra -Wshadow -Wimplicit-function-declaration
TGT_CFLAGS	+= -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
TGT_CFLAGS	+= -fno-common -ffunction-sections -fdata-sections
TGT_CFLAGS      += -MD -Wall -Wundef
TGT_CFLAGS      += $(DEFS)

###############################################################################
# Linker flags

TGT_LDFLAGS		+= --static -nostartfiles
TGT_LDFLAGS		+= -T$(LDSCRIPT)
TGT_LDFLAGS		+= $(ARCH_FLAGS)
TGT_LDFLAGS		+= -Wl,-Map=$(*).map
TGT_LDFLAGS		+= -Wl,--gc-sections
ifeq ($(V),99)
TGT_LDFLAGS		+= -Wl,--print-gc-sections
endif

###############################################################################
# Used libraries

LDLIBS		+= -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

###############################################################################
###############################################################################
###############################################################################

.SUFFIXES: .elf .bin .hex .srec .list .map .images
.SECONDEXPANSION:
.SECONDARY:

all: stylecheck elf 

elf: $(BIN_DIR)/$(TARGET).elf
bin: $(BIN_DIR)/$(TARGET).bin
hex: $(BIN_DIR)/$(TARGET).hex
srec: $(BIN_DIR)/$(TARGET).srec
list: $(BIN_DIR)/$(TARGET).list

images: $(BIN_DIR)/$(TARGET).images
flash: $(BIN_DIR)/$(TARGET).flash

# Either verify the user provided LDSCRIPT exists, or generate it.
$(LDSCRIPT):
    ifeq (,$(wildcard $(LDSCRIPT)))
        $(error Unable to find specified linker script: $(LDSCRIPT))
    endif

# Define a helper macro for debugging make errors online
# you can type "make print-OPENCM3_DIR" and it will show you
# how that ended up being resolved by all of the included
# makefiles.
print-%:
	@echo $*=$($*)

$(BIN_DIR)/%.images: $(BIN_DIR)/%.bin $(BIN_DIR)/%.hex $(BIN_DIR)/%.srec $(BIN_DIR)/%.lst $(BIN_DIR)/%.map
	@printf "*** $* images generated ***\n"

$(BIN_DIR)/%.bin: $(BIN_DIR)/%.elf
	@printf "  OBJCOPY $(*).bin\n"
	$(Q)$(OBJCOPY) -Obinary $(BIN_DIR)/$(*).elf $(BIN_DIR)/$(*).bin

$(BIN_DIR)/%.hex: $(BIN_DIR)/%.elf
	@printf "  OBJCOPY $(*).hex\n"
	$(Q)$(OBJCOPY) -Oihex $(BIN_DIR)/$(*).elf $(BIN_DIR)/$(*).hex

$(BIN_DIR)/%.srec: $(BIN_DIR)/%.elf
	@printf "  OBJCOPY $(*).srec\n"
	$(Q)$(OBJCOPY) -Osrec $(BIN_DIR)/$(*).elf $(BIN_DIR)/$(*).srec

$(BIN_DIR)/%.lst: $(BIN_DIR)/%.elf
	@printf "  OBJDUMP $(*).lst\n"
	$(Q)$(OBJDUMP) -S $(BIN_DIR)/$(*).elf > $(BIN_DIR)/$(*).lst

$(BIN_DIR)/%.elf $(BIN_DIR)/%.map: $(OBJS) $(LDSCRIPT) bin_dir
	@printf "  LD      $(*).elf\n"
	$(Q)$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(BIN_DIR)/$(*).elf

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c libopencm3 obj_dir src/hoptable.h 
	@printf "  CC      $(*).c\n"
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) -o $(OBJECT_DIR)/$(*).o -c $(SOURCE_DIR)/$(*).c

src/hoptable.h: 
	python ./scripts/generate_hoptable.py > src/hoptable.h

clean:
	@#printf "  CLEAN\n"
	$(Q)$(RM) $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d $(BIN_DIR)/*.elf $(BIN_DIR)*.bin $(BIN_DIR)*.hex $(BIN_DIR)/*.srec $(BIN_DIR)/*.lst $(BIN_DIR)/*.map generated.* ${OBJS} ${OBJS:%.o:%.d}

stylecheck: $(HEADER_FILES) $(SOURCE_FILES_FOUND)
	@./stylecheck/cpplint.py --filter=-build/include,-build/storage_class,-readability/casting,-runtime/arrays --extensions="h,c" --root=src --linelength=120 $(HEADER_FILES) $(SOURCE_FILES_FOUND) || true

%.stlink-flash: %.bin
	@printf "  FLASH  $<\n"
	$(STFLASH) write $(*).bin 0x8000000


bin_dir:
	@mkdir -p ${BIN_DIR}

obj_dir:
	@mkdir -p ${OBJECT_DIR}
	@mkdir -p ${OBJECT_DIR}/eeprom_emulation

ifeq ($(STLINK_PORT),)
ifeq ($(BMP_PORT),)
ifeq ($(OOCD_FILE),)
%.flash: %.elf
	@printf "  FLASH   $<\n"
	(echo "halt; program $(realpath $(*).elf) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f interface/$(OOCD_INTERFACE).cfg \
		-f target/$(OOCD_TARGET).cfg \
		-c "program $(*).elf verify reset exit" \
		$(NULL)
else
%.flash: %.elf
	@printf "  FLASH   $<\n"
	(echo "halt; program $(realpath $(*).elf) verify reset" | nc -4 localhost 4444 2>/dev/null) || \
		$(OOCD) -f $(OOCD_FILE) \
		-c "program $(*).elf verify reset exit" \
		$(NULL)
endif
else
%.flash: %.elf
	@printf "  GDB   $(*).elf (flash)\n"
	$(GDB) --batch \
		   -ex 'target extended-remote $(BMP_PORT)' \
		   -x $(SCRIPT_DIR)/black_magic_probe_flash.scr \
		   $(*).elf
endif
else
%.flash: %.elf
	@printf "  GDB   $(*).elf (flash)\n"
	$(GDB) --batch \
		   -ex 'target extended-remote $(STLINK_PORT)' \
		   -x $(SCRIPT_DIR)/stlink_flash.scr \
		   $(*).elf
endif
sterase : 
	st-flash erase

stflasherase : sterase stflash

stflash : $(BIN_DIR)/$(TARGET).bin
	st-flash --reset write $(BIN_DIR)/$(TARGET).bin 0x8000000 


dfu : $(BIN_DIR)/$(TARGET).bin
	$(DFU_UTIL) -a 0 -D $(BIN_DIR)/$(TARGET).bin -s 0x08000000:leave -R


libopencm3 : libopencm3/lib/libopencm3_stm32f0.a submodules

libopencm3/lib/libopencm3_stm32f0.a: 
	$(MAKE) -C libopencm3

#git submodules handling
submodules:
	@git submodule update --init -- libopencm3


.PHONY: images clean stylecheck styleclean elf bin hex srec list submodules bin_dir obj_dir stylecheck

-include $(OBJS:.o=.d)
