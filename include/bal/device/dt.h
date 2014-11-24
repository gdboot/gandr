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

#ifndef BAL_DEVICE_DT_H
#define BAL_DEVICE_DT_H
#include <gd_bal.h>
#include <gd_tree.h>

typedef struct dt_property {
    RB_ENTRY(dt_property) rbnode;
    size_t value_len;
    const char * name;
    void *value;
} *dt_property_t;

typedef struct dt_node {
    RB_ENTRY(dt_node)                    rbnode;
    RB_HEAD(dt_nodes, dt_node)           children;
    RB_HEAD(dt_properties, dt_property)  properties;
    struct dt_node                      *parent;
    gd_device_t                          bound_device;
    const char                          *name;
} *dt_node_t;

int dt_property_cmp(dt_property_t lhs, dt_property_t rhs);
int dt_node_cmp(dt_node_t lhs, dt_node_t rhs);

RB_PROTOTYPE(dt_properties, dt_property, rbnode, dt_property_cmp)
RB_PROTOTYPE(dt_nodes,      dt_node,     rbnode, dt_node_cmp)

dt_node_t     dt_node_alloc(dt_node_t parent, const char *name);
dt_node_t     dt_node_find_child(dt_node_t parent, const char *name);
dt_property_t dt_node_find_property(dt_node_t node, const char *name);
bool          dt_node_has_property(dt_node_t node, const char *name);
const void   *dt_node_get_property(dt_node_t node, const char *name);
dt_property_t dt_node_set_property(
    dt_node_t   node,
    const char *name,
    const void *value,
    size_t      len);
bool          dt_node_get_reg_range(
    dt_node_t  node,
    unsigned   idx,
    uintptr_t *ptr,
    size_t    *sz);

unsigned      dt_node_get_reg_count(
    dt_node_t  node);
uint32_t      dt_node_get_address_cells(dt_node_t node);
uint32_t      dt_node_get_size_cells(dt_node_t node);
uint32_t      dt_property_get_uint32(dt_property_t prop);
uint64_t      dt_property_get_uint64(dt_property_t prop);
dt_node_t     dt_root_node(void);

typedef gd_device_t (*dt_driver_attach)(dt_node_t node);

/*! Structure which identifies a device driver. Place
 *  BAL_FDT_DECLARE_DEVICE_DRIVER in your driver code in order to declare a
 *  driver for a device
 */
struct dt_driver {
    /*! DeviceTree compatible string */
    const char *compatible;

    /*! Driver attach function */
    dt_driver_attach attach;
};

#define DT_DECLARE_DEVICE_DRIVER(sym, name, func) \
struct dt_driver sym \
    __attribute__((section("dt_drivers"), unused)) = \
    { name, func };

int dt_base_ioctl(gd_device_t dev, unsigned ioctl, va_list *pap);
void dt_platform_init_fdt(void *fdt);

#endif
