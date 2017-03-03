#ifndef MACROS_H__
#define MACROS_H__

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define GPIO_RCC(GPIO)  (((0x14) << 5) + (17 + ((GPIO - PERIPH_BASE_AHB2)/0x0400)))

#define DEFINE_TO_STR(x) #x
#define DEFINE_TO_STR_VAL(x) DEFINE_TO_STR(x)

#define min(a, b) (((a) < (b)) ? (a):(b))
#define max(a, b) (((a) > (b)) ? (a):(b))

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

#endif  // MACROS_H__
