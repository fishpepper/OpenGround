CFLAGS  = -O1 -g
ASFLAGS = -g
GENERIC_SRCS  = main.c delay.c assert.c led.c lcd.c screen.c console.c font.c debug.c io.c adc.c sound.c timeout.c touch.c cc2500.c spi.c
#debug.c assert.c clocksource.c timeout.c wdt.c delay.c frsky.c spi.c cc25xx.c
#GENERIC_SRCS += io.c storage.c failsafe.c ppm.c adc.c sbus.c apa102.c soft_spi.c soft_serial.c telemetry.c

include Makefile.board

all  : board
