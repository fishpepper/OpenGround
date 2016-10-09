CFLAGS  = -O1 -g
ASFLAGS = -g
GENERIC_SRCS  = main.c delay.c assert.c led.c lcd.c screen.c console.c font.c debug.c io.c
GENERIC_SRCS += adc.c sound.c timeout.c touch.c cc2500.c spi.c frsky.c storage.c wdt.c gui.c
GENERIC_SRCS += eeprom.c telemetry.c fifo.c

include Makefile.board

all  : board
