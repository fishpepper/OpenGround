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

#include "usb.h"
#include "debug.h"
#include "adc.h"
#include "delay.h"
#include "macros.h"
#include "console.h"
#include "screen.h"
#include "config.h"
#include "cc2500.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>

static bool usb_init_done;
static uint32_t usb_systick_count;

// internal data storage
static usbd_device *usbd_dev;

// internal functions
static void usb_init_rcc(void);
static void usb_init_core(void);
// static void usb_loop(void);
// void usb_handle_data(void);
static void usb_send_data(void);

void usb_init(void) {
    debug("usb: init\n"); debug_flush();

    usb_systick_count = 0;
    usb_init_done = false;

    usb_init_rcc();

    // set up usb core
    usb_init_core();
}

void usb_init_rcc(void) {
    // USB I/Os are on port A
    rcc_periph_clock_enable(GPIO_RCC(USB_GPIO));

    // USB clock
    rcc_periph_clock_enable(RCC_USB);
}



const struct usb_device_descriptor usb_dev_descr = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x0483,
    .idProduct = 0x5710,
    .bcdDevice = 0x0200,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

static const uint8_t usb_hid_report_descriptor[] = {
     0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
     0x09, 0x04,  // USAGE (Joystick)

     0xa1, 0x01,  // COLLECTION (Application)
     0xa1, 0x00,  // COLLECTION (Physical)

     0x05, 0x09,  // USAGE_PAGE (Button)
     0x19, 0x01,  // USAGE_MINIMUM (Button 1)
     0x29, 0x08,  // USAGE_MAXIMUM (Button 8)
     0x15, 0x00,  // LOGICAL_MINIMUM (0)
     0x25, 0x01,  // LOGICAL_MAXIMUM (1)
     0x95, 0x08,  // REPORT_COUNT (8)
     0x75, 0x01,  // REPORT_SIZE (1)
     0x81, 0x02,  // INPUT (Data,Var,Abs)

     0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
     0x15, 0x00,  // LOGICAL_MINIMUM (0)
     0x26, 0x00, 0x19,  // LOGICAL_MAXIMUM (6400 = 0x1900)
     0x75, 0x10,  // REPORT_SIZE

     0x09, 0x30,  // USAGE (X)
     0x09, 0x31,  // USAGE (Y)
     0x09, 0x32,  // USAGE (Z)
     0x09, 0x33,  // USAGE (Rx)
     0x09, 0x34,  // USAGE (Ry)
     0x09, 0x35,  // USAGE (Rz)
     0x09, 0x36,  // USAGE (Throttle)
     0x09, 0x37,  // USAGE (Rudder)
     0x95, 0x08,  // REPORT_COUNT
     0x81, 0x82,  // INPUT (Data,Var,Abs,Vol)

     0xc0,  // END_COLLECTION
     0xc0  // END_COLLECTION
};

static const struct {
    struct usb_hid_descriptor hid_descriptor;
    struct {
        uint8_t bReportDescriptorType;
        uint16_t wDescriptorLength;
    } __attribute__((packed)) hid_report;
} __attribute__((packed)) hid_function = {
    .hid_descriptor = {
        .bLength = sizeof(hid_function),
        .bDescriptorType = USB_DT_HID,
        .bcdHID = 0x0100,
        .bCountryCode = 0,
        .bNumDescriptors = 1,
    },
    .hid_report = {
        .bReportDescriptorType = USB_DT_REPORT,
        .wDescriptorLength = sizeof(usb_hid_report_descriptor),
    }
};

const struct usb_endpoint_descriptor usb_hid_endpoint = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x81,
    .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
    .wMaxPacketSize = 17,
    .bInterval = 0x02,
};

const struct usb_interface_descriptor usb_hid_iface = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = USB_CLASS_HID,
    .bInterfaceSubClass = 0,  // boot
    .bInterfaceProtocol = 0,  // mouse
    .iInterface = 0,

    .endpoint = &usb_hid_endpoint,

    .extra = &hid_function,
    .extralen = sizeof(hid_function),
};

const struct usb_interface usb_ifaces[] = {{
    .num_altsetting = 1,
    .altsetting = &usb_hid_iface,
}};

const struct usb_config_descriptor usb_config = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 0,
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0xC0,
    .bMaxPower = 0x32,

    .interface = usb_ifaces,
};

static const char *usb_strings[] = {
    "fishpepper.de",
    "OpenGround",
    "HID Joystick",
};

// buffer to be used for control requests
uint8_t usbd_control_buffer[128];

static int usb_hid_control_request(usbd_device * UNUSED(dev),
                                   struct usb_setup_data *req,
                                   uint8_t **buf, uint16_t *len,
                                   void (** complete)(usbd_device *,
                                       struct usb_setup_data *)
                                   ) {
    (void)complete;  // disable unused param warning

    if ((req->bmRequestType != 0x81) ||
       (req->bRequest != USB_REQ_GET_DESCRIPTOR) ||
       (req->wValue != 0x2200)) {
        return 0;
    }

    // handle the HID report descriptor
    *buf = (uint8_t *)usb_hid_report_descriptor;
    *len = sizeof(usb_hid_report_descriptor);

    usb_init_done = true;
    return 1;
}

static void usb_hid_set_config(usbd_device *dev, uint16_t UNUSED(wValue)) {
    // set up endpoint
    usbd_ep_setup(dev, 0x81, USB_ENDPOINT_ATTR_INTERRUPT, 4, 0);

    // register hid callback
    usbd_register_control_callback(
                dev,
                USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
                USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
                usb_hid_control_request);

    #if 0
    // / systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
    /* SysTick interrupt every N clock pulses: set reload to N-1 */
    systick_set_reload(99999);
    systick_interrupt_enable();
    systick_counter_enable();
    #endif  // 0
}


void usb_init_core(void) {
    /*
     * This is a somewhat common cheap hack to trigger device re-enumeration
     * on startup.  Assuming a fixed external pullup on D+ (F0 seems to have it internally),
     * (For USB-FS) setting the pin to output, and driving it explicitly low effectively
     * "removes" the pullup.  The subsequent USB init will "take over" the
     * pin, and it will appear as a proper pullup to the host.
     * The magic delay is somewhat arbitrary, no guarantees on USBIF
     * compliance here, but "it works" in most places.
     */
    gpio_mode_setup(USB_GPIO, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, USB_DP_PIN);
    gpio_set_output_options(USB_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, USB_DP_PIN);
    gpio_clear(USB_GPIO, USB_DP_PIN);

    for (unsigned i = 0; i < 800000; i++) {
        __asm__("nop");
    }

    usbd_dev = usbd_init(&st_usbfs_v2_usb_driver,
                         &usb_dev_descr ,
                         &usb_config,
                         usb_strings,
                         3,
                         usbd_control_buffer,
                         sizeof(usbd_control_buffer));

    usbd_register_set_config_callback(usbd_dev, usb_hid_set_config);
}

void usb_handle_data(void) {
    usbd_poll(usbd_dev);
}

void usb_handle_systick(void) {
    // only send data if usb is initialized
    if (!usb_init_done) {
        return;
    }

    // systick is called with 0.1ms
    // we want to send data with 100Hz (=10ms)
    if (usb_systick_count++ >= (100-1)) {
        usb_systick_count = 0;
        usb_send_data();
    }
}

bool usb_enabled(void) {
    return usb_init_done;
}

void usb_send_data(void) {
    // static uint16_t adc_data = 0;
    static uint8_t buf[1 + 16];

    // buttons
    buf[0] = 0;  // adc_data~(gpio_get(GPIOB, BUTTONS_PINS) >> BUTTONS_SHIFT);

    // sticks
    for (unsigned int i = 0; i < 8; i++) {
        uint16_t res = 3200 + adc_get_channel_rescaled(i);
        buf[1 + i * 2] = res & 0xff;
        buf[1 + i * 2 + 1] = res >> 8;
    }

    usbd_ep_write_packet(usbd_dev, 0x81, buf, sizeof(buf));
}
