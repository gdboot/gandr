#ifndef PORTIO_H
#define PORTIO_H
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile( "out %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t val;
    asm volatile( "in %1, %0" : "a"(val) : "Nd"(port) );
    return val;
}

static inline void outw(uint16_t port, uint16_t val)
{
    asm volatile( "out %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint16_t inb(uint16_t port)
{
    uint16_t val;
    asm volatile( "in %1, %0" : "a"(val) : "Nd"(port) );
    return val;
}

static inline void outl(uint16_t port, uint32_t val)
{
    asm volatile( "out %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t val;
    asm volatile( "in %1, %0" : "a"(val) : "Nd"(port) );
    return val;
}

#ifdef __amd64__
static inline void outq(uint16_t port, uint64_t val)
{
    asm volatile( "out %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint64_t inq(uint16_t port)
{
    uint64_t val;
    asm volatile( "in %1, %0" : "a"(val) : "Nd"(port) );
    return val;
}
#endif

#endif
