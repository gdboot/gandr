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
 
#ifndef PORTIO_H
#define PORTIO_H
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val)
{
    __asm volatile( "out %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm volatile( "in %1, %0" : "=a"(val) : "Nd"(port) );
    return val;
}

static inline void outw(uint16_t port, uint16_t val)
{
    __asm volatile( "out %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t val;
    __asm volatile( "in %1, %0" : "=a"(val) : "Nd"(port) );
    return val;
}

static inline void outl(uint16_t port, uint32_t val)
{
    __asm volatile( "out %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t val;
    __asm volatile( "in %1, %0" : "=a"(val) : "Nd"(port) );
    return val;
}

#ifdef __amd64__
static inline void outq(uint16_t port, uint64_t val)
{
    __asm volatile( "out %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint64_t inq(uint16_t port)
{
    uint64_t val;
    __asm volatile( "in %1, %0" : "=a"(val) : "Nd"(port) );
    return val;
}
#endif

#endif
