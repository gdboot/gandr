#ifndef GIO_GENERIC_H
#define GIO_GENERIC_H
#include <stdint.h>
#include <stddef.h>

/* GIO implementation for platforms which use only MMIO. These functions are
 * thin wrappers around memory accesses.
 */

typedef struct {
	volatile void *base;
} gio_addr;

#define GIO_MMIO_ADDR(_addr) ((gio_addr){ (void*) (_addr) })

static inline void gio_write8_offset(gio_addr addr, size_t offs, uint8_t val)
{
	volatile uint8_t *ptr = ((volatile uint8_t*) addr.base) + offs;
	*ptr = val;
}

static inline void gio_write16_offset(gio_addr addr, size_t offs, uint16_t val)
{
	volatile uint16_t *ptr = (volatile uint16_t*) (((volatile char*)addr.base) + offs);
	*ptr = val;
}

static inline void gio_write32_offset(gio_addr addr, size_t offs, uint32_t val)
{
	volatile uint32_t *ptr = (volatile uint32_t*) (((volatile char*)addr.base) + offs);
	*ptr = val;
}

static inline uint8_t gio_read8_offset(gio_addr addr, size_t offs)
{
	volatile uint8_t *ptr = ((volatile uint8_t*) addr.base) + offs;
	return *ptr;
}

static inline uint16_t gio_read16_offset(gio_addr addr, size_t offs)
{
	volatile uint16_t *ptr = (volatile uint16_t*) (((volatile char*)addr.base) + offs);
	return *ptr;
}

static inline uint32_t gio_read32_offset(gio_addr addr, size_t offs)
{
	volatile uint32_t *ptr = (volatile uint32_t*) (((volatile char*)addr.base) + offs);
	return *ptr;
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
