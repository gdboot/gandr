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

#ifndef GIO_ARCH_H
#define GIO_ARCH_H
#include <stdint.h>
#include <stddef.h>
#include <bal/portio.h>

/* GIO implementation for x86 */

/* Always 8 bytes, even on i386
 *
 * Discriminator is MSB of 8 byte address, i.e. type field. AMD64 has 
 * architecturally defined paddr limit of 56-bits, which leaves the upper 8 bits
 * for us to play with (and anyway we could only address 48-bits of RAM). Store
 * a tag there.
 *
 * For efficiency reasons, let type == 0 be MMIO. x86 only has one other I/O 
 * type, port IO, which is type != 0.
 */
typedef union {
    uint64_t _ensure_size_and_pad;

    volatile void *mmio_base;

    struct {
        uint16_t portio_base;
        uint8_t  _portio_pad[5];
        uint8_t  type;
    };
} gio_addr;

#define GIO_MMIO_ADDR(_addr)   ((gio_addr){ .mmio_base = (void*) (_addr) })
#define GIO_PORTIO_ADDR(_addr) ((gio_addr){ .portio_base = (_addr), .type = 1})

static inline void gio_write8_offset(gio_addr addr, size_t offs, uint8_t val)
{
    if(addr.type == 0) {
        volatile uint8_t *ptr = ((volatile uint8_t*) addr.mmio_base) + offs;
        *ptr = val;
    } else outb(addr.portio_base + offs, val);
}

static inline void gio_write16_offset(gio_addr addr, size_t offs, uint16_t val)
{
    if(addr.type == 0) {
        volatile uint16_t *ptr = (volatile uint16_t*) (((volatile char*)addr.mmio_base) + offs);
        *ptr = val;
    } else outw(addr.portio_base + offs, val);
}

static inline void gio_write32_offset(gio_addr addr, size_t offs, uint32_t val)
{
    if(addr.type == 0) {
        volatile uint32_t *ptr = (volatile uint32_t*) (((volatile char*)addr.mmio_base) + offs);
        *ptr = val;
    } else outl(addr.portio_base + offs, val);
}

static inline uint8_t gio_read8_offset(gio_addr addr, size_t offs)
{
    if(addr.type == 0) {
        volatile uint8_t *ptr = ((volatile uint8_t*) addr.mmio_base) + offs;
        return *ptr;
    } else return inb(addr.portio_base + offs);
}

static inline uint16_t gio_read16_offset(gio_addr addr, size_t offs)
{
    if(addr.type == 0) {
        volatile uint16_t *ptr = (volatile uint16_t*) (((volatile char*)addr.mmio_base) + offs);
        return *ptr;
    } else return inw(addr.portio_base + offs);
}

static inline uint32_t gio_read32_offset(gio_addr addr, size_t offs)
{
    if(addr.type == 0) {
        volatile uint32_t *ptr = (volatile uint32_t*) (((volatile char*)addr.mmio_base) + offs);
        return *ptr;
    } else return inl(addr.portio_base + offs);
}

static inline void gio_write8_index(gio_addr addr, size_t idx, uint8_t val)
{ gio_write8_offset(addr, idx, val); }

static inline void gio_write16_index(gio_addr addr, size_t idx, uint16_t val)
{ gio_write16_offset(addr, idx * 2, val); }

static inline void gio_write32_index(gio_addr addr, size_t idx, uint32_t val)
{ gio_write32_offset(addr, idx * 4, val); }

static inline uint8_t gio_read8_index(gio_addr addr, size_t idx)
{ return gio_read8_offset(addr, idx); }

static inline uint16_t gio_read16_index(gio_addr addr, size_t idx)
{ return gio_read16_offset(addr, idx * 2); }

static inline uint32_t gio_read32_index(gio_addr addr, size_t idx)
{ return gio_read32_offset(addr, idx * 4); }

static inline void gio_write8(gio_addr addr, uint8_t val)
{ gio_write8_offset(addr, 0, val); }

static inline void gio_write16(gio_addr addr, uint16_t val)
{ gio_write16_offset(addr, 0, val); }

static inline void gio_write32(gio_addr addr, uint32_t val)
{ gio_write32_offset(addr, 0, val); }

static inline uint8_t gio_read8(gio_addr addr)
{ return gio_read8_offset(addr, 0); }

static inline uint16_t gio_read16(gio_addr addr)
{ return gio_read16_offset(addr, 0); }

static inline uint32_t gio_read32(gio_addr addr)
{ return gio_read32_offset(addr, 0); }

#endif
