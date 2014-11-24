/* Copyright © 2014, Owen Shepherd
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef GD_BAL_DEVICE_ARM_PL011_H
#define GD_BAL_DEVICE_ARM_PL011_H
#include <stdint.h>
#include <gd_bal.h>
#include <bal/gio.h>
#include <bal/device/dt.h>

typedef struct {
    GD_DEVICE;
    //! DT node
    dt_node_t node;
    //! Base address
    gio_addr base;
    //! BRG clock speed
    uint32_t brg_clock;
} arm_pl011_dev;

#define DEFINE_ARM_PL011(name, base_, brg_clock_) \
    arm_pl011_dev name = { .base = base_, .brg_clock = brg_clock_}

void arm_pl011_init(arm_pl011_dev *dev);

#endif
