/* Copyright © 2014, Owen Shepherd
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

#include "kickstart.h"
#include <elfload.h>
#include <gd_elf.h>
#include <stddef.h>
#include <stdint.h>

extern char __startspace[];
uintptr_t mem_mark = (uintptr_t) &__startspace;

static void align(uintptr_t *v, size_t align)
{
    align -= 1;

    *v = (*v + align) & (~align);
}

static void fail(const char *reason)
{
    printf(ESC_BOLD "%s\n" ESC_NORMAL "Kickstart failed. Halted.\n", reason);

    for(;;);
}

void ks_main(void)
{
    puts("Gandr ATF Kickstart!");

    unsigned magic   = DT_GANDR_ATFKICKSTART_VERSION;
    unsigned version = 0;
    void *dtb_addr   = ks_load_dtb();
    void *entrypoint = ks_load_bal();

    printf("Preparing to go to %p (dtb=%p)\n", entrypoint, dtb_addr);

    ks_go(magic, version, dtb_addr, entrypoint);
}

void *ks_load_dtb(void)
{
    printf("Loading DTB...");
    int fd = io_open("platform.dtb");
    if (fd == -1) fail(" platform.dtb not found");

    char *dtbstart = (char*) mem_mark;
    unsigned dtb_size = io_len(fd);

    mem_mark += dtb_size;

    for(unsigned i = 0; i < dtb_size; i += 1024) {
        uputc('.');
        unsigned to_read = (i + 1024) < dtb_size ? 1024 : dtb_size - i;
        int rv = io_read(fd, dtbstart + i, to_read);
        if (rv != (int) to_read)
            fail(" I/O error");
    }

    io_close(fd);

    printf(ESC_BOLD " Done\n" ESC_NORMAL);

    return dtbstart;
}

typedef struct {
    el_ctx el;
    int fd;
} ks_el_ctx;


static bool ks_pread(struct el_ctx *ctx, void *dest, size_t nb, size_t offset)
{
    int rv;
    ks_el_ctx *ksc = (ks_el_ctx*) ctx;
    if ((rv = io_seek(ksc->fd, offset))) {
        printf(" Seek failed (to 0x%X); %d", (unsigned) offset, rv);
        return false;
    }

    int read = io_read(ksc->fd, dest, nb);

    if (read != (int) nb) {
        printf(" Read failed (0x%X)", (unsigned) nb);
        return false;
    }

    return true;
}

static void check_el(el_status res, const char *action)
{
    if (res == EL_OK) return;

    puts(ESC_BOLD " ELF loader error" ESC_NORMAL);
    printf("While %s: ", action);
    switch (res) {
        case EL_EIO:           fail("I/O error");
        case EL_ENOMEM:        fail("Out of memory");
        case EL_NOTELF:        fail("Not an ELF file");
        case EL_WRONGBITS:     fail("Wrong bitness");
        case EL_WRONGENDIAN:   fail("Wrong endian");
        case EL_WRONGARCH:     fail("Wrong architecture");
        case EL_WRONGOS:       fail("Wrong OS");
        case EL_NOTEXEC:       fail("Not executable");
        case EL_NODYN:         fail("Not dynamic");
        case EL_BADREL:        fail("Bad relocation");
        default:
            printf("(%d)", res);
            fail("Unknown");
    }
}

static void *ks_el_alloc(
    el_ctx *ctx,
    Elf_Addr phys,
    Elf_Addr virt,
    Elf_Addr size)
{
    return (void*) phys;
}

void *ks_load_bal(void)
{
    printf("Loading Gandr BAL...");

    ks_el_ctx ctx;
    ctx.el.pread = ks_pread;
    ctx.fd = io_open("gd_atf64.elf");
    if (ctx.fd == -1)
        fail(" gd_atf64.elf not found");

    int len = io_len(ctx.fd);
    printf(" (%d bytes)", len);

    check_el(el_init(&ctx.el), "initialising");
    printf( " init");

    Elf_Dyn dyn;
    check_el(el_finddyn(&ctx.el, &dyn, DT_GANDR_ATFKICKSTART_VERSION),
        "finding ABI tag");

    if (dyn.d_tag == 0)
        fail(" no ABI tag");

printf( " abi");

    /* No need to check value - we only speak v0 */

    align(&mem_mark, ctx.el.align);
    ctx.el.base_load_paddr = ctx.el.base_load_vaddr = mem_mark;

    check_el(el_load(&ctx.el, ks_el_alloc), "loading");
    printf( " loaded @ %p ", (void*) ctx.el.base_load_paddr);

    check_el(el_relocate(&ctx.el), "perfomring relocations");
    printf( " reloc");

    io_close(ctx.fd);

    puts(ESC_BOLD " done" ESC_NORMAL);

    return (void*) (ctx.el.ehdr.e_entry + mem_mark);
}
