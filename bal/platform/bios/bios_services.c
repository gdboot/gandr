/* Copyright © 2013-2014, Owen Shepherd
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

#include <bal/bios_services.h>
#include <string.h>

#ifdef __i386__
 #define R(x) "%%e" #x
#else
 #define R(x) "%%r" #x
#endif

uint8_t scratch_buffer[4096];
struct bios_service_table *bios_services;

static inline void* push(void *restrict *restrict psp, const void *restrict src, size_t len)
{
    *psp = ((char*) *psp) - len;
    return memcpy(*psp, src, len);
}

void bios_far_call(uint32_t address, struct bios_registers *regs, const void *stack_data, unsigned stack_size)
{
    void *sp = (void*) (uintptr_t) bios_services->rm_stack;

    uint32_t tmp = 0;

    // Reserve space on stack for our esp
    void **ret_info_ptr = push(&sp, &tmp, sizeof tmp);

    // Push data onto the stack
    push(&sp, stack_data, stack_size);

    // Push address
    push(&sp, &address, sizeof address);

    // Copy registers onto the stack
    push(&sp, regs, sizeof *regs);

    // Jump into the transition vector
    __asm volatile(
#ifdef __i386__
        // Save all regs using pushad
        "pushal\n"
#endif
        // (On long mode we rely on preservation of r8+ to satifsy the compiler)


        // Use this rather than push to produce a 32-bit compatible far return
        // frame
        "sub $8, %%esp\n"
        "mov %%cs, %%eax\n"
        "mov %%eax, 4(" R(sp) ")\n"
        "mov $1f, %%eax\n"
        "mov %%eax, 0(" R(sp) ")\n"

        // Stash esp on the real mode stack
        "mov %%esp, %[ret_info]\n"

        // Switch to real mode stack and jump in
        "mov %[sp], " R(sp) "\n"
        "data16 ljmp *%[far_call_tvec]\n"

        "1:\n"
#ifdef __i386__
        "popal\n"
#endif

        : [ret_info]        "=m"    (*ret_info_ptr)
        : [sp]              "r"     (sp)
        , [far_call_tvec]   "m"     (bios_services->far_call_ptr)
        : "cc", "memory"
#ifdef __amd64__
        , "rax", "rbx", "rcx", "rdx"
        , "rsi", "rdi", "rbp"
#else
        , "eax"
#endif
    );

    // Copy back the registers
    memcpy(regs, sp, sizeof *regs);
}

void bios_int_call(uint8_t num, struct bios_registers *regs)
{
    void *sp = (void*) (uintptr_t) bios_services->rm_stack;

    uint32_t tmp = 0;

    // Reserve space on stack for our esp
    void ** ret_info_ptr = push(&sp, &tmp, sizeof tmp);

    // Copy registers onto the stack
    push(&sp, regs, sizeof *regs);

    // Jump into the transition vector
    __asm volatile(
#ifdef __i386__
        // Save all regs using pushad
        "pushal\n"
#endif
        // (On long mode we rely on preservation of r8+ to satifsy the compiler)

        // Use this rather than push to produce a 32-bit compatible far return
        // frame
        "sub $8, %%esp\n"
        "mov %%cs, %%ebx\n"
        "mov %%ebx, 4(" R(sp) ")\n"
        "mov $1f, %%ebx\n"
        "mov %%ebx, 0(" R(sp) ")\n"

        // Stash esp on the real mode stack
        "mov %%esp, %[ret_info]\n"

        // Switch to real mode stack and jump in
        "mov %[sp], " R(sp) "\n"
        "data16 ljmp *%[int_call_tvec]\n"

        "1:\n"
#ifdef __i386__
        "popal\n"
#endif

        : [ret_info]        "=m"    (*ret_info_ptr)
        , [num]             "+a"    (num)
        : [sp]              "r"     (sp)
        , [int_call_tvec]   "m"     (bios_services->int_call_ptr)
        : "cc", "memory"
#ifdef __amd64__
        , "rbx", "rcx", "rdx"
        , "rsi", "rdi", "rbp"
#else
        , "ebx"
#endif
    );

    // Copy back the registers
    memcpy(regs, sp, sizeof *regs);
}

