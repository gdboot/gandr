/* Copyright © 2013-2014, Owen Shepherd.
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

#ifndef GD_BAL_BIOS_SERVICES_H
#define GD_BAL_BIOS_SERVICES_H

#define RM_CS 0x00
#define RM_DS 0x00
#define PM_CS16 0x08
#define PM_DS16 0x10
#define PM_CS32 0x18
#define PM_DS32 0x20
#define PM_CS64 0x28
#define PM_DS64 PM_DS32

#ifndef __ASSEMBLER__
#include <assert.h>
#include <stddef.h>
#include <stddef.h>
#include <stdint.h>

struct bios_registers {
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eflags;
};

typedef enum {
    carry_flag = (1 << 0),
    parity_flag = (1 << 2),
    adjust_flag = (1 << 4),
    zero_flag = (1 << 6),
    sign_flag = (1 << 7),
    trap_flag = (1 << 8),
    interrupt_enable_flag = (1 << 9),
    direction_flag = (1 << 10),
    overflow_flag = (1 << 11)
} eflags;

struct bios_service_table {
    uint32_t magic;         //!< 'BIOS'
    uint32_t size;          //!< sizeof(*this);

    uint32_t rm_stack;      //!< Pointer to real mode stack

    uint32_t far_call_ptr;  //!< Far pointer to far call entrypoint
    uint32_t int_call_ptr;  //!< Far pointer to int call entrypoint
};

extern struct bios_service_table *bios_services;

void bios_far_call(uint32_t address, struct bios_registers *regs, const void* stack_data, unsigned stack_size);
void bios_int_call(uint8_t  num,     struct bios_registers *regs);

extern volatile uint8_t scratch_buffer[4096];

/*! Get a real mode segment from the given linear address */
static inline uint16_t rm_seg_from_ptr(const void *p)
{
    uintptr_t v = (uintptr_t) p;
    return (v >> 4);
}

/*! Get a real mode offset from the given linear address */
static inline uint16_t rm_offset_from_ptr(const void *p)
{
    uintptr_t v = (uintptr_t) p;
    return (v & 0x000F);
}

/*! Get a real mode far pointer from the given linear address */
static inline uint32_t rm_far_from_ptr(const void *p)
{
    uintptr_t v = (uintptr_t) p;
    uint32_t fp = (v << 12 & 0xFFFF0000) | (v & 0x0F);
    return fp;
}

/*! Convert a real mode far pointer to a linear address */
static inline void* rm_ptr_from_far(uint32_t fp) {
    uintptr_t v = (fp >> 12 & 0x000FFFF0) + (fp & 0xFFFF);
    return (void*) v;
}


#endif

#endif
