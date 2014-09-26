/* Copyright © 2013-2014, Owen Shepherd & Shikhin Sethi
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

#ifndef GD_BAL_H
#define GD_BAL_H
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#define GD_CONCAT_(a, b) a##b
#define GD_CONCAT(a, b) GD_CONCAT_(a, b)

#define GD_UNPACK_(...) __VA_ARGS__
#define GD_UNPACK(x)    GD_UNPACK_ x

typedef struct gd_device {
    int (*ioctl)(struct gd_device *, unsigned, ...);
} *gd_device_t;

int gd_ioctl(gd_device_t, unsigned, ...);

#include "gd_ioctl.inc"

#define GD_BEGIN_IOCTL_MAP(_type, _name)                                       \
static int _name(gd_device_t dev_, unsigned ioctl, ...)     \
{                                                                              \
    int rv = 0;                                                                \
    _type dev = (_type) dev_;                                                  \
    va_list ap, *pap = &ap;                                                    \
    va_start(ap, ioctl);                                                       \
postForward:                                                                   \
    switch(ioctl) {                                                            \
    case GD_FORWARD_IOCTL: {                                                   \
        ioctl = va_arg(*pap, unsigned);                                        \
        pap   = va_arg(*pap, va_list *);                                       \
        goto postForward;                                                      \
    }

#define GD_END_IOCTL_MAP()                                                     \
    default:                                                                   \
        /*errno = EINVAL;*/                                                    \
        rv = -1;                                                               \
    }                                                                          \
    va_end(ap);                                                                \
    return rv;                                                                 \
}

#define GD_END_IOCTL_MAP_FORWARD(_forwardTo)                                   \
    default:                                                                   \
        rv = (_forwardTo)(dev_, GD_FORWARD_IOCTL, ioctl, pap);                 \
    }                                                                          \
    va_end(ap);                                                                \
    return rv;                                                                 \
}

/*! A Gandr table header.
 *
 * All tables are aligned to 8 byte boundaries. When processing multiple
 * sequential tables, e.g. the boot information directory, the next table is
 * found at the address
 *      ((const char*)&table) + (table.length + 7 & ~7)
 *
 * You should verify that the size of each table is <b>at least</b> that
 * specified by the specification. You <b>must</b> accept longer tables,
 * ignoring their extra content, to facilitate future specification revisions.
 *
 * Any table with an identifier starting with a lower case letter is boot
 * services only and a kernel loader can discard it when preparing the data
 * to pass to the kernel.
 */
typedef struct {
    /*! Identifier of the table */
    uint32_t id;

    /*! Length of the table, in bytes, including this header */
    uint32_t length;
} gd_table;

#define GD_TABLE_ID(a, b, c, d) \
    ((uint32_t) ((a << 24) | (b << 16) | (c << 8) | (d)))

/*! Copies the table ID to the passed buffer, in "natural reading order"
 *  The passed buffer <b>must</b> at least 5 bytes in size, as this function
 *  null terminates it
 */
static inline void gd_table_id_to_string(char *buf, const gd_table *table)
{
    buf[0] = (char) (table->id >> 24 & 0xFF);
    buf[1] = (char) (table->id >> 16 & 0xFF);
    buf[2] = (char) (table->id >> 8  & 0xFF);
    buf[3] = (char) (table->id       & 0xFF);
    buf[4] = 0;
}

/*! Returns a pointer to the next table */
static inline gd_table *gd_next_table(const gd_table *table)
{
    return (gd_table*) (((char*) table) + ((table->length + 7) & ~7));
}

/*! Defined types of a memory region.
 *
 *  Type codes 0x0000_0000 - 0x7FFF_FFFF are taken from the UEFI specification.
 *  Type codes 0x8000_0000 - 0xFFFF_FFFF are Gandr defined
 */
typedef enum {
    EfiReservedMemoryType = 0,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,

    /*! ARM EABI and similar make enum sizes dependent upon their max value
     *  Force this enum to 32-bits
     */
    gd_memory_force_size = INT_MAX
} gd_memory_type;

/*! A memory map entry. This format is aligned with, but not the same as that
 *  used by the UEFI specification. In particular, UEFI uses a pageCount member;
 *  Gandr uses a size member;
 */
typedef struct {
    /*! The type of the memory region. */
    gd_memory_type type;

    /*! Physical address at which the region begins */
    uint64_t physical_start;

    /*! Virtual address at which the loader mapped this region, if it was mapped */
    uint64_t virtual_start;

    /*! Size, in bytes, of the region */
    uint64_t size;

    /*! Attribute bits
     *  \see gd_memory_map_attribute
     */
    uint64_t attributes;
} gd_memory_map_entry;

/*! System memory map
 */
typedef struct {
    gd_table header;
    gd_memory_map_entry entries[];
} gd_memory_map_table;
#define GD_MEMORY_MAP_TABLE_ID GD_TABLE_ID('M', 'M', 'A', 'P')

void mmap_remove_entry(gd_memory_map_table *table, size_t idx);
void mmap_add_entry(gd_memory_map_table *table, gd_memory_map_entry entry);

#endif
