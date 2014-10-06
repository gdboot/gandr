/* Copyright © 2013, Owen Shepherd
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

#ifndef GIO_H
#define GIO_H
#include <stdint.h>
#include <bal/gio_arch.h>

/*! \defgroup gio Generic I/O
 *  GIO provides an interface which abstracts the details of the various I/O 
 *  methods possible on the target platform from the driver in question.
 *
 *  The primary example of this is the i386 distinction between port and memory
 *  mapped I/O. 
 *
 *  On all platforms, a \p gio_addr structure can be initialized from a memory
 *  mapped I/O address by using \p GIO_MMIO_ADDR. Other platforms may define
 *  other initialization mechanisms. For example, i386 defines 
 *  \p GIO_PORTIO_ADDR.
 *
 * \{
 */  

/***** Platform dependent portion *********************************************/

//! Write the 8-bit value \p val to (\p addr + \p offs)
void gio_write8_offset(gio_addr addr, size_t offs, uint8_t val);
//! Write the 16-bit value \p val to (\p addr + \p offs)
void gio_write16_offset(gio_addr addr, size_t offs, uint16_t val);
//! Write the 32-bit value \p val to (\p addr + \p offs)
void gio_write32_offset(gio_addr addr, size_t offs, uint32_t val);

//! Read an 8-bit value from (\p addr + \p offs)
uint8_t gio_read8_offset(gio_addr addr, size_t offs);
//! Read a 16-bit value from (\p addr + \p offs)
uint16_t gio_read16_offset(gio_addr addr, size_t offs);
//! Read a 32-bit value from (\p addr + \p offs)
uint32_t gio_read32_offset(gio_addr addr, size_t offs);

//! Write an 8-bit value \p val to (\p addr + \p idx * sizeof(uint8_t))
void gio_write8_index(gio_addr addr, size_t idx, uint8_t val);
//! Write a 16-bit value \p val to (\p addr + \p idx * sizeof(uint16_t))
void gio_write16_index(gio_addr addr, size_t idx, uint16_t val);
//! Write a 32-bit value \p val to (\p addr + \p idx * sizeof(uint32_t))
void gio_write32_index(gio_addr addr, size_t idx, uint32_t val);

//! Read an 8-bit value from (\p addr + \p idx * sizeof(uint8_t))
uint8_t gio_read8_index(gio_addr addr, size_t idx);
//! Read a 16-bit value from (\p addr + \p idx * sizeof(uint16_t))
uint16_t gio_read16_index(gio_addr addr, size_t idx);
//! Read a 32-bit value from (\p addr + \p idx * sizeof(uint32_t))
uint32_t gio_read32_index(gio_addr addr, size_t idx);

//! Write an 8-bit value \p val to \p addr
void gio_write8(gio_addr addr, uint8_t val);
//! Write a 16-bit value \p val to \p addr
void gio_write16(gio_addr addr, uint16_t val);
//! Write a 32-bit value \p val to \p addr
void gio_write32(gio_addr addr, uint32_t val);

//! Read an 8-bit value from \p addr
uint8_t gio_read8(gio_addr addr);
//! Read a 16-bit value from \p addr
uint16_t gio_read16(gio_addr addr);
//! Read a 32-bit value from \p addr
uint32_t gio_read32(gio_addr addr);

/***** Platform independent portion *******************************************/

/*! Reinterpret \p value as a \p size bit quantity, then write the resulting
 *  value to (\p base + \p index * sizeof(\p value)), where the sizeof operator
 * is computed after reinterpretation
 */
void gio_write_index(size_t size, gio_addr base, size_t index, size_t value);
/*! Read from the address (\p base + \p index * sizeof(uintN_t)), where the N
 *  in uintN_t is defined to be one of (8, 16, 32) as given by the size 
 *  parameter, where the size of the read in bits is given by \p size, and 
 *  return the result
 */
size_t gio_read_index(size_t size, gio_addr base, size_t index);

// XXX: gio_[read|write]_offset

/*! Write \p value to the address \p base wherein \p value is reinterpreted as
 *  a value of one of the types (uint8_t, uint16_t, uint32_t) in which the bit
 *  width is given by the value of \p size
 */
void gio_write(size_t size, gio_addr base, size_t value);

/*! Read from the address \p base wherein the read performed is 8, 16 or 32 bits
 *  and given by size
 */
size_t gio_read(size_t size, gio_addr base);

/*! \} */

#endif
