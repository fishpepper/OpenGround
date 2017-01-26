CFLAGS  = -O1 -g
ASFLAGS = -g
SRC_FILES  = main.c delay.c assert.c led.c lcd.c screen.c console.c font.c debug.c io.c
SRC_FILES += adc.c sound.c timeout.c touch.c cc2500.c spi.c frsky.c storage.c wdt.c gui.c
SRC_FILES += eeprom.c telemetry.c fifo.c crc16.c config.c

# add src path
GENERIC_SRCS = $(SRC_FILES:%.c=src/%.c)

include Makefile.board

all  : board
