/*
 Copyright (C) 2016 fishpepper
 Copyright (C) 2008 by Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307
 USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.*/

/* Functional Description: Generic FIFO library for deeply
   embedded system. See the unit tests for usage examples.
   This library only uses a byte sized chunk.
   This library is designed for use in Interrupt Service Routines
   and so is declared as "static inline" */

#ifndef FIFO_H__
#define FIFO_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    volatile unsigned head;      /* first byte of data */
    volatile unsigned tail;     /* last byte of data */
    volatile uint8_t *buffer; /* block of memory or array of data */
    unsigned buffer_len;     /* length of the data */
} fifo_buffer_t;

bool fifo_empty(fifo_buffer_t const *b);

uint8_t fifo_peek(fifo_buffer_t const *b);

uint8_t fifo_get(fifo_buffer_t * b);

bool fifo_put(fifo_buffer_t * b, uint8_t data_byte);

/* note: buffer_len must be a power of two */
void fifo_init(fifo_buffer_t * b, volatile uint8_t *buffer, unsigned buffer_len);

#endif  // FIFO_H__
