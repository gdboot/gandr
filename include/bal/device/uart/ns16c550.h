/* Copyright © 2013, Owen Shepherd
 * 
 * Permission to use, copy, modify, and/or distribute this software for any 
 * purpose with or without fee is hereby granted, provided that the above 
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH 
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, 
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef NS16C550_H
#define NS16C550_H
#include <stdint.h>
#include <bal/gio.h>

typedef struct {
    //! Base address
    gio_addr base;
    //! FIFO size
    uint16_t fifo_size;
    //! Known available FIFO space
    uint16_t fifo_space;

    //! Register width
    uint8_t reg_width;
} ns16c550_dev;

void ns16c550_init(ns16c550_dev *dev);
void ns16c550_txbyte(ns16c550_dev *dev, char c);
char ns16c550_rxbyte(ns16c550_dev *dev);

#endif
