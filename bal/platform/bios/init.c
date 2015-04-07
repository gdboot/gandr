/* Copyright © 2013-2014, Shikhin Sethi
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

#include <stdio.h>
#include <_PDCLIB_io.h>
#include <bal/bios_services.h>
#include <bal/bios_console.h>
#include <gd_common.h>
#include <bal/mmap.h>
#include <platform/bios/bal/mmap.h>
#include <bal/tables.h>
#include <bal/vbe.h>

struct bios_console_dev bios_console;

__asm__(
".globl _start\n"
"_start:\n"
#ifdef __x86_64__
"        mov %edi, %edi\n"
#else
"        push %edi\n"
#endif
"        call __start\n"
"        jmp .\n"
);

void __start(struct bios_service_table *pbios_services);
void __start(struct bios_service_table *pbios_services)
{
    bios_services = pbios_services;
    __asm("xchg %bx, %bx");
    bios_console_init(&bios_console);
    stdout->handle.pointer = &bios_console.dev;

    /* Initialize the memory map, find all the tables. */
    mmap_init();
    tables_init();
    vbe_init();

    /*gd_memory_map_entry mmap[100]; size_t nentries = 100, key = 0;
    mmap_get(&mmap, nentries, &nentries, &key);
    for (size_t i = 0; i < nentries; i++) {
        printf("Entry %d: %llx -> %llx, %d\n", i, mmap[i].physical_start,
               mmap[i].physical_start + mmap[i].size,
               mmap[i].type);
    }*/

    extern gd_rsdt_pointer_table rsdt_pointer;
    extern gd_pc_pointer_table pc_pointer;

    if (rsdt_pointer.header.length) {
        printf("RSDT: %x\nXSDT: %llx\n", rsdt_pointer.rsdt_address, rsdt_pointer.xsdt_address);
    }

    if (pc_pointer.header.length) {
        printf("MPS: %c%c%c%c\nSMBIOS: %x\n", pc_pointer.mpfp.signature[0],
                                              pc_pointer.mpfp.signature[1],
                                              pc_pointer.mpfp.signature[2],
                                              pc_pointer.mpfp.signature[3], pc_pointer.smbios_entry_point_address);
    }
    for(;;);
}
