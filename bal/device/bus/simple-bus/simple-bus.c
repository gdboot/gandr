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
#include <bal/device/bus/simple-bus.h>
#include <bal/device/dt.h>
#include <bal/misc.h>
#include <libfdt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static int sb_bus_get_child_reg_addr(
    struct simple_bus_dev* self,
    gd_device_t child,
    unsigned idx,
    gio_addr *addr,
    size_t *len)
{
    int rv = 0;
    dt_node_t node;

    if ((rv = gd_device_get_dt_node(child, &node)))
        return rv;

    uintptr_t base_addr;
    size_t base_size;

    if (dt_node_get_reg_range(node, idx, &base_addr, &base_size)) {
        *addr = GIO_MMIO_ADDR(base_addr);
        *len  = base_size;

        return 0;
    } else {
        return ERANGE;
    }
}

static int sb_bus_get_child_reg_count(
    struct simple_bus_dev *self,
    gd_device_t child,
    unsigned *count)
{
    int rv;
    dt_node_t node;
    if ((rv = gd_device_get_dt_node(child, &node)))
        return rv;

    *count = dt_node_get_reg_count(node);
    return 0;
}


static GD_BEGIN_IOCTL_MAP(struct simple_bus_dev *, simple_bus_ioctl)
    GD_MAP_BUS_GET_CHILD_REG_COUNT_IOCTL(sb_bus_get_child_reg_count)
    GD_MAP_BUS_GET_CHILD_REG_ADDR_IOCTL(sb_bus_get_child_reg_addr)
GD_END_IOCTL_MAP_FORWARD_BASE(dt_base_ioctl)

static gd_device_t sb_driver_attach(
    dt_node_t node)
{
    struct simple_bus_dev *self = malloc(sizeof *self);
    if (!self)
        return NULL;
    self->ioctl = simple_bus_ioctl;
    self->node  = node;

    unsigned addr_cells = dt_node_get_address_cells(node);
    unsigned size_cells = dt_node_get_size_cells(node);

    if (addr_cells < 1 || addr_cells > 2) {
        size_t needed;
        char path[256];
        gd_device_get_path(self, path, 256, &needed);

        panic("simple-bus \"%s\" with %d address cells",
            path, addr_cells);
    }

    if (size_cells < 1 || size_cells > 2) {
        size_t needed;
        char path[256];
        gd_device_get_path(self, path, 256, &needed);

        panic("simple-bus \"%s\" with %d size cells",
            path, size_cells);
    }

    return &self->dev;
}

DT_DECLARE_DEVICE_DRIVER(simple_bus_dt_dev, "simple-bus", sb_driver_attach)
