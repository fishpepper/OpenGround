#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

//font indices
#define FONT_LENGTH       0
#define FONT_FIXED_WIDTH  2
#define FONT_HEIGHT       3
#define FONT_FIRST_CHAR   4
#define FONT_CHAR_COUNT   5
#define FONT_WIDTH_TABLE  6

//helpers
#define font_is_fixed_width(_f)  ((_f[FONT_LENGTH] == 0) && (_f[FONT_LENGTH+1] < 2))
#define font_is_nopad_fixed_font(_f)  ((_f[FONT_LENGTH] == 0) && (_f[FONT_LENGTH+1] == 1))

extern const uint8_t font_system5x7[];
extern const uint8_t font_tomthumb3x5[];

#endif // __FONT_H__
