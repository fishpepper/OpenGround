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


#include "delay.h"

void delay_init(void) {
    // nothing to do
}


inline void delay_us(uint32_t us) {
    // based on https:// github.com/leaflabs/libmaple
    us *= 8;

    // fudge for function call overhead
    us--;
    us--;
    us--;
    us--;
    us--;
    us--;
    us--;
    asm volatile(".syntax unified           \n\t"
         "   mov r0, %[us]          \n\t"
                 "1: subs r0, #1            \n\t"
                 "   bhi 1b                 \n\t"
                 ".syntax divided           \n\t"
                 :
                 : [us] "r" (us)
                 : "r0");
}

