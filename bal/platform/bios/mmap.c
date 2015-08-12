/* Copyright © 2014, Shikhin Sethi
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

#include <gd_common.h>
#include <bal/bios_services.h>
#include <bal/mmap.h>
#include <platform/bios/bal/mmap.h>
#include <bal/portio.h>
#include <stdbool.h>

/*! Type conversion from ACPI to gd* types. */
gd_memory_type acpi_to_gd[] = {
    gd_unusable_memory,      /* unused */
    gd_conventional_memory,  /* address_range_memory */
    gd_unusable_memory,      /* address_range_reserved */
    gd_acpi_reclaim_memory,  /* address_range_reclaimable */
    gd_acpi_memory_nvs,      /* address_range_nvs */
    gd_unusable_memory,      /* address_range_unusable */
    gd_unusable_memory       /* address_range_disabled */
};

static void add_acpi_range(struct address_range range)
{
    if (!(range.attributes & entry_present))
        return;

    gd_memory_map_entry entry;
    entry.physical_start = range.physical_start;
    entry.virtual_start = 0;
    entry.size = range.size;

    /* TODO: handle attributes. */
    entry.attributes = 0;

    if (range.type < (sizeof acpi_to_gd / sizeof (gd_memory_type)))
        entry.type = acpi_to_gd[range.type];
    else
        entry.type = gd_unusable_memory;

    mmap_add_entry(entry);
}

/*! Returns true on success, otherwise false. */
static bool try_e820()
{
    struct bios_registers regs = { 
        .eax = 0x0000E820,
        .ebx = 0x00000000,  /* continuation value */
        .ecx = 24,          /* size of output buffer */
        .edx = 0x534D4150   /* SMAP */
    };
   volatile struct address_range range = { .attributes = entry_present };

    regs.es = rm_seg_from_ptr((void*) &range);
    regs.edi = rm_offset_from_ptr((void*) &range);
    bios_int_call(0x15, &regs);

    // Carry flag after first call means unsupported.
    if ((regs.eflags & carry_flag) || 
        (regs.eax != 0x534D4150)  ||
        (regs.ecx < 20))
        return false;

    add_acpi_range(range);

    do {
        regs.eax = 0x0000E820; regs.ecx = 24; regs.edx = 0x534D4150;
        regs.es = rm_seg_from_ptr((void*) &range);
        regs.edi = rm_offset_from_ptr((void*) &range);

        range.attributes = entry_present;
        bios_int_call(0x15, &regs);

        // List finished.
        if (regs.eflags & carry_flag)
            return true;

        // If invalid entry, reset.
        if ((regs.eax != 0x534D4150) || (regs.ecx < 20)) {
            mmap_clean();
            return false;
        }

        add_acpi_range(range);
    } while (regs.ebx);

    return true;
}

/*! Returns true on success, otherwise false.
 *      \param eax is the exact 0xe8X1 function.
 */
static bool try_e8x1(uint32_t eax)
{
    struct bios_registers regs = { .eax = eax };
    bios_int_call(0x15, &regs);
    if (regs.eflags & carry_flag)
        return false;

    if (eax == 0xE801) {
        regs.eax &= 0xFFFF;
        regs.ebx &= 0xFFFF;
        regs.ecx &= 0xFFFF;
        regs.edx &= 0xFFFF;
    }

    /*! Linux uses the CX/DX pair, but reverts to AX/BX if they're
     *  zero. */
    if (!regs.ecx) {
        regs.ecx = regs.eax;
        regs.edx = regs.ebx;
    }

    gd_memory_map_entry entries[] = {
        { .physical_start = 0x100000,
          .size = regs.ecx * 1024,
          .type = gd_conventional_memory },

        { .physical_start = 0x1000000,
          .size = regs.edx * 1024 * 64,   /* 64K blocks */
          .type = gd_conventional_memory }
    };

    mmap_add_entry(entries[0]);
    mmap_add_entry(entries[1]);
    return true;
}

static bool try_c7()
{
    // Use 0xC0 to figure out if 0xC7 is supported.
    struct bios_registers regs = { .eax = 0xC0 };
    bios_int_call(0x15, &regs);

    if ((regs.eflags & carry_flag) || (regs.eax & 0xFF00))
        return false;

    uint8_t *rom_table = (uint8_t*)(uintptr_t) (regs.es * 0x10 + (regs.ebx & 0xFFFF));

    // Bit 4 of second feature byte at offset 6.
    if (!(rom_table[6] & (1 << 4)))
        return false;

    struct {
        uint16_t word;
        uint32_t local_memory_1m, local_memory_16m;
        uint32_t system_memory_1m, system_memory_16m;
        uint32_t cacheable_memory_1m, cacheable_memory_16m;
        uint32_t memory_1m, memory_16m;
        uint16_t free_block_c0000; uint16_t free_block_size;
        uint32_t reserved;
    } __attribute__((packed)) *c7_memory_map;

    c7_memory_map = (void*)(uintptr_t) (regs.ds * 0x10 + (regs.esi & 0xFFFF));
    gd_memory_map_entry entries[] = {
        { .physical_start = 0x100000,
          .size = c7_memory_map->memory_1m * 1024,
          .type = gd_conventional_memory },

        { .physical_start = 0x1000000,
          .size = c7_memory_map->memory_16m * 1024,
          .type = gd_conventional_memory }
    };
    mmap_add_entry(entries[0]);
    mmap_add_entry(entries[1]);
    return true;
}

static uint64_t get_extended_memory()
{
    struct bios_registers regs = { .eax = 0xDA88 };
    bios_int_call(0x15, &regs);

    if (!(regs.eflags & carry_flag)) {
        /* success */
        return (((regs.ecx & 0xFF) << 16) | (regs.ebx & 0xFFFF)) * 1024;
    }

    regs.eax = 0x8800;
    bios_int_call(0x15, &regs);

    /* AH=0x88 isn't reliable with carry flag setting,
       so if AH is either 0x86 (unsupported function) or 0x80
       (invalid command) after call, treat as error. */
    if (!(regs.eflags & carry_flag) &&
        ((regs.eax & 0xFF00) != 0x8600) &&
        ((regs.eax & 0xFF00) != 0x8000)) {
        return (regs.eax & 0xFFFF) * 1024;
    }

    regs.eax = 0x8A00;
    bios_int_call(0x15, &regs);

    if (!(regs.eflags & carry_flag)) {
        return (((regs.edx & 0xFFFF) << 16) | (regs.eax & 0xFFFF)) * 1024;
    }

    // CMOS also stores information.
    uint8_t low, high;
    outb(0x70, 0x30);
    low = inb(0x71);
    outb(0x70, 0x31);
    high = inb(0x71);

    return (low | (high << 8)) * 1024;
}

/*! Get the amount of base memory, as reported by int 0x12. */
uint32_t get_base_memory()
{
    static uint32_t base_memory = 0;
    if (!base_memory) {
        struct bios_registers regs = { 0 };
        bios_int_call(0x12, &regs);
        base_memory = regs.eax * 1024;
    }

    return base_memory;
}

void mmap_init()
{
    if (try_e820() == false) {
        /* fallback */
        /* todo: does this need entries for EBDA or BIOS region? */
        gd_memory_map_entry entries[] = {
            { .physical_start = 0x00000000,
              .size = get_base_memory(),
              .type = gd_conventional_memory },

            // ISA hole.
            { .physical_start = 0xF00000,
              .size = 0x100000,
              .type = gd_unusable_memory }
        };
        for (size_t i = 0; i < sizeof entries / sizeof (gd_memory_map_entry); i++)
            mmap_add_entry(entries[i]);

        if((try_e8x1(0xe881) == false) &&
           (try_e8x1(0xe801) == false) &&
           (try_c7() == false)) {
            gd_memory_map_entry entry = {
                .physical_start = 0x100000,
                .size = get_extended_memory(),
                .type = gd_conventional_memory
            };

            mmap_add_entry(entry);
        }
    }

    /* todo: pxe */
    /* todo: self */
    gd_memory_map_entry low_mem = {
        .physical_start = 0x0000,
        /*! IVT and BDA (till 0x501). */
        .size = 0x1000,
        .type = gd_unusable_memory
    };

    // TODO: make this proper.
    gd_memory_map_entry gandr_mem = {
        .physical_start = 0x8000,
        .size = (0x30000 - 0x8000),
        .type = gd_loader_code
    };

    mmap_add_entry(low_mem); mmap_add_entry(gandr_mem);
}
