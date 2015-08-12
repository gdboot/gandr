/* Copyright © 2014, Shikhin Sethi.
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

#ifndef ARCH_STRING_H
#define ARCH_STRING_H

#include <stdint.h>

#if defined(__i386__) || defined(__x86_64__)

/*! memset 8-bitwise. */
static inline void* memset8(void *dest, uint8_t c, size_t n)
{
    if (n) __asm__ __volatile__("rep stosb" :: "c"(n), "D"(dest), "a"((uint32_t)c));
    return dest;
}

/*! memset 16-bitwise. */
static inline void* memset16(void *dest, uint16_t c, size_t n)
{
    if (n) __asm__ __volatile__("rep stosw" :: "c"(n), "D"(dest), "a"((uint32_t)c));
    return dest;
}

/*! memset 32-bitwise. */
static inline void* memset32(void *dest, uint32_t c, size_t n)
{
    if (n) __asm__ __volatile__("rep stosl" :: "c"(n), "D"(dest), "a"(c));
    return dest;
}

/*! memcpy 8-bitwise. */
static inline void* memcpy8(void *dest, const void* src, size_t n)
{
    if (n) __asm__ __volatile__("rep movsb" :: "c"(n), "D"(dest), "S"(src));
    return dest;
}

/*! memcpy 16-bitwise. */
static inline void* memcpy16(void *dest, const void* src, size_t n)
{
    if (n) __asm__ __volatile__("rep movsw" :: "c"(n), "D"(dest), "S"(src));
    return dest;
}

/*! memscpy 32-bitwise. */
static inline void* memcpy32(void *dest, const void* src, size_t n)
{
    if (n) __asm__ __volatile__("rep movsl" :: "c"(n), "D"(dest), "S"(src));
    return dest;
}

#else

#error Bit specific mem* functions not defined for this arch.

#endif

#endif
