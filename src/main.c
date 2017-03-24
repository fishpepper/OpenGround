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

#include "main.h"
#include "clocksource.h"
#include "timeout.h"
#include "config.h"
#include "delay.h"
#include "sound.h"
#include "led.h"
#include "io.h"
#include "lcd.h"
#include "debug.h"
#include "screen.h"
#include "console.h"
#include "adc.h"
#include "sound.h"
#include "touch.h"
#include "cc2500.h"
#include "frsky.h"
#include "storage.h"
#include "wdt.h"
#include "gui.h"
#include "eeprom.h"
#include "usb.h"


#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>


// Define our function pointer
void (*bootloader_jump)(void);

static void handle_bootloader_request(void) {
    // Check if we should go into bootloader mode.
    //
    // Set the main stack pointer __set_MSP() to its default value.  The default
    // value of the main stack pointer is found by looking at the default value
    // in the System Memory start address. Do this in IAR View -> Memory.  I
    // tried this and it showed address: 0x200014A8 which I then tried here.
    // The IAR compiler complained that it was out of range.  After some
    // research, I found the following from "The STM32 Cortex-M0 Programming
    // Manual":
    //         Main Stack Pointer (MSP)(reset value). On reset, the processor
    //         loads the MSP with the value from address 0x00000000.
    //
    // So I then looked at the default value at address 0x0 and it was 0x20002250
    //
    // Note that 0x1fffC800 is "System Memory" start address for STM32 F0xx
    //
    if ( *((uint32_t *)0x20003FF0) == 0xDEADBEEF ) {
         *((uint32_t *)0x20003FF0) =  0xCAFEFEED;  // Reset our trigger
        // 0x1fffC800 is "System Memory" start address for STM32 F0xx
        // point the PC to the System Memory reset vector (+4)
        bootloader_jump = (void (*)(void)) (*((uint32_t *) 0x1fffC804));
        bootloader_jump();
        while (1) {}
    }
}

#ifdef USB_ENABLED

int main(void) {
    rcc_clock_setup_in_hsi_out_48mhz();

    rcc_periph_clock_enable(RCC_GPIOA);
    /*
     * This is a somewhat common cheap hack to trigger device re-enumeration
     * on startup.  Assuming a fixed external pullup on D+, (For USB-FS)
     * setting the pin to output, and driving it explicitly low effectively
     * "removes" the pullup.  The subsequent USB init will "take over" the
     * pin, and it will appear as a proper pullup to the host.
     * The magic delay is somewhat arbitrary, no guarantees on USBIF
     * compliance here, but "it works" in most places.
     */
    // set led pin as output
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO12);
    gpio_clear(GPIOA, GPIO12);

    for (unsigned i = 0; i < 800000; i++) {
        __asm__("nop");
    }

    usbd_dev = usbd_init(&st_usbfs_v2_usb_driver, &dev_descr,
                         &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
    usbd_register_set_config_callback(usbd_dev, hid_set_config);

    while (1)
        usbd_poll(usbd_dev);
}

void sys_tick_handler(void) {
    /*static int x = 0;
    static int dir = 1;
    uint8_t buf[4] = {0, 0, 0, 0};

    buf[1] = dir;
    x += dir;
    if (x > 30)
        dir = -dir;
    if (x < -30)
        dir = -dir;

    usbd_ep_write_packet(usbd_dev, 0x81, buf, 4);*/
    static uint16_t adc_data = 0;
    static uint8_t buf[1 + 16];
    static uint8_t channels[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

        for (unsigned int i = 0; i < 8; i++) {
            uint16_t res = adc_data++;
            buf[1 + i * 2] = res & 0xff;
            buf[1 + i * 2 + 1] = res >> 8;
        }

        buf[0] = 0;  // adc_data~(gpio_get(GPIOB, BUTTONS_PINS) >> BUTTONS_SHIFT);
    usbd_ep_write_packet(usbd_dev, 0x81, buf, sizeof(buf));
}




#else
int main(void) {
    // if this was a reboot with bootloader request enter
    // internal rom bootloader
    handle_bootloader_request();

    // init crystal osc & set clock options
    clocksource_init();


    config_init();

    delay_init();


    led_init();


//    wdt_init();

    io_init();
    timeout_init();


    lcd_init();


    lcd_show_logo();
    screen_init();


    console_init();


    debug_init();


    adc_init();
    sound_init();


    touch_init();
    eeprom_init();
    storage_init();

    frsky_init();

    usb_init();

    /// screen_test();
    // touch_test();
    // adc_test();
    gui_init();


    debug("main: init done.\n"); debug_flush();

    gui_loop();

    while (1) {
        // adc_test();
        // io_powerdown_test();
        // debug("ABCDEFGHIJKLMN\ni   .   !\n"); debug_flush();
        // while (1);
        //
    }
}

#endif  // 0
