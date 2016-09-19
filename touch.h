#ifndef __TOUCH__H_
#define __TOUCH__H_
#include <stdint.h>
#include "config.h"
#include "stm32f0xx.h"

void touch_init(void);
void touch_test(void);
static void touch_deinit_i2c(void);
static void touch_init_i2c_mode(void);
static void touch_init_i2c_gpio(void);
static void touch_init_i2c_rcc(void);
static void touch_init_i2c_free_bus(void);
static uint32_t touch_i2c_read(uint8_t address, uint8_t *data, uint8_t len);
static uint8_t touch_i2c_read_byte(uint8_t reg);
static void touch_ft6236_debug_info(void);
static void touch_init_isr(void);
static void touch_ft6236_init(void);


typedef struct {
    uint8_t event_id;
    uint16_t x;
    uint16_t y;
} touch_event_t;

touch_event_t touch_get_and_clear_last_event(void);

void EXTI4_15_IRQHandler(void);

#define TOUCH_FT6236_MAX_TOUCH_POINTS     2

#define TOUCH_FT6236_REG_TH_GROUP         0x80
#define TOUCH_FT6236_REG_PERIODACTIVE     0x88
#define TOUCH_FT6236_REG_LIB_VER_H        0xa1
#define TOUCH_FT6236_REG_LIB_VER_L        0xa2
#define TOUCH_FT6236_REG_CIPHER           0xa3
#define TOUCH_FT6236_REG_FIRMID           0xa6
#define TOUCH_FT6236_REG_FOCALTECH_ID     0xa8
#define TOUCH_FT6236_REG_RELEASE_CODE_ID  0xaf

#define TOUCH_FT6236_EVENT_PRESS_DOWN     0
#define TOUCH_FT6236_EVENT_LIFT_UP        1
#define TOUCH_FT6236_EVENT_CONTACT        2
#define TOUCH_FT6236_EVENT_NO_EVENT       3

#define TOUCH_RESET_HI() { TOUCH_RESET_GPIO->BSRR = (TOUCH_RESET_PIN); }
#define TOUCH_RESET_LO() { TOUCH_RESET_GPIO->BRR = (TOUCH_RESET_PIN); }

#define TOUCH_FT6236_GESTURE_MOVE_FLAG   0x10
#define TOUCH_FT6236_GESTURE_MOVE_UP     0x10
#define TOUCH_FT6236_GESTURE_MOVE_RIGHT  0x14
#define TOUCH_FT6236_GESTURE_MOVE_DOWN   0x18
#define TOUCH_FT6236_GESTURE_MOVE_LEFT   0x1C
#define TOUCH_FT6236_GESTURE_ZOOM_IN     0x48
#define TOUCH_FT6236_GESTURE_ZOOM_OUT    0x49
#define TOUCH_FT6236_GESTURE_NONE        0x00

#define TOUCH_GESTURE_UP    ((TOUCH_FT6236_GESTURE_MOVE_UP    & 0x0F)+1)
#define TOUCH_GESTURE_DOWN  ((TOUCH_FT6236_GESTURE_MOVE_DOWN  & 0x0F)+1)
#define TOUCH_GESTURE_LEFT  ((TOUCH_FT6236_GESTURE_MOVE_LEFT  & 0x0F)+1)
#define TOUCH_GESTURE_RIGHT ((TOUCH_FT6236_GESTURE_MOVE_RIGHT & 0x0F)+1)
#define TOUCH_GESTURE_CLICK (0x80)


struct __attribute__ ((__packed__)) touch_ft6236_touchpoint {
    union {
        uint8_t xhi;
        uint8_t event;
    };

    uint8_t xlo;

    union {
        uint8_t yhi;
        uint8_t id;
    };

    uint8_t ylo;
    uint8_t weight;
    uint8_t misc;
};

//this packet represents the register map as read from offset 0
typedef struct __attribute__ ((__packed__)){
    uint8_t dev_mode;
    uint8_t gest_id;
    uint8_t touches;
    struct touch_ft6236_touchpoint points[TOUCH_FT6236_MAX_TOUCH_POINTS];
} touch_ft6236_packet_t;

#endif // __TOUCH__H_
