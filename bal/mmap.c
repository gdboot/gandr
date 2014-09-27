/* Copyright © 2014, Shikhin Sethi
 * 
 * Permission to use, copy, modify, and/or distribute this software for any 
 * purpose with or without fee is hereby granted, provided that the above 
 * copyright notice and this permission notice appear in all copies.
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
#include <string.h>

static gd_memory_type overlap_precedence[] = {
    /*! The overlapping region is promoted to the one that comes earlier. */
    gd_unusable_memory, gd_pal_code, gd_mmio_port_space, gd_mmio,
    gd_acpi_memory_nvs, gd_runtime_services_code, gd_runtime_services_data,
    gd_acpi_reclaim_memory, gd_loader_code, gd_loader_data, gd_boot_services_code,
    gd_boot_services_data, gd_conventional_memory, gd_reserved_memory_type
};

/*! Returns the type with higher precedence. */
static gd_memory_type higher_precedence(gd_memory_type a, gd_memory_type b)
{
    for (size_t i = 0; i < sizeof overlap_precedence/sizeof (gd_memory_type); i++)
    {
        if (overlap_precedence[i] == a) 
            return a;
        else if (overlap_precedence[i] == b)
            return b;
    }

    // Unusable.
    return gd_unusable_memory;
}

static void merge_adjacent(gd_memory_map_table *table, size_t first_idx)
{
    if ((table->entries[first_idx].physical_start + table->entries[first_idx].size 
        == table->entries[first_idx + 1].physical_start) &&
        (table->entries[first_idx].type == table->entries[first_idx + 1].type)) {
        table->entries[first_idx].size += table->entries[first_idx + 1].size;
        table->entries[first_idx].attributes |= table->entries[first_idx + 1].attributes;
        mmap_remove_entry(table, first_idx + 1);
    }
}

static void fix_overlap(gd_memory_map_table *table,
                        size_t first_idx)
{
    gd_memory_map_entry *first = &table->entries[first_idx];
    gd_memory_map_entry *second = &table->entries[first_idx + 1];

    if ((first->physical_start + first->size) <= second->physical_start)
        return;

    gd_memory_map_entry new_entry;

    // Partition in three, where the new_entry is the right-most part.
    uint64_t first_physical_end = first->physical_start + first->size;
    uint64_t second_physical_end = second->physical_start + second->size;
    if (first_physical_end < second_physical_end) {
        new_entry.physical_start = first_physical_end;
        new_entry.size = second_physical_end - first_physical_end;
        new_entry.type = second->type; new_entry.attributes = second->attributes;
    } else {
        new_entry.physical_start = second_physical_end;
        new_entry.size = first_physical_end - second_physical_end;
        new_entry.type = first->type; new_entry.attributes = first->attributes;
    }

    first->size = second->physical_start - first->physical_start;
    second->size = new_entry.physical_start - second->physical_start;
    /* TODO: handle attributes. */
    second->attributes = first->attributes | second->attributes;
    second->type = higher_precedence(first->type, second->type);

    if (!second->size) {
        mmap_remove_entry(table, first_idx + 1);
    }
    if (!first->size) {
        mmap_remove_entry(table, first_idx);
    }

    mmap_add_entry(table, new_entry);
}

void mmap_remove_entry(gd_memory_map_table *table, size_t idx)
{
    for(size_t i = idx; i < (mmap_get_size(table) - 1); i++) {
        memcpy(&table->entries[i], &table->entries[i + 1], sizeof (gd_memory_map_entry));
    }
    table->header.length -= sizeof (gd_memory_map_entry);
}

void mmap_add_entry(gd_memory_map_table *table, gd_memory_map_entry entry)
{
    if (!entry.size)
        return;

    size_t idx;
    for (idx = mmap_get_size(table); idx > 0; idx--) {
        // Sort in increasing order.
        if (table->entries[idx - 1].physical_start <= entry.physical_start)
            break;

        memcpy(&table->entries[idx], &table->entries[idx - 1], sizeof (gd_memory_map_entry));
    }

    table->header.length += sizeof (gd_memory_map_entry);
    memcpy(&table->entries[idx], &entry, sizeof (gd_memory_map_entry));

    /* TODO: look after attributes. */
    if (idx) {
        merge_adjacent(table, idx - 1);
        fix_overlap(table, idx - 1);
    }
    if (idx < (mmap_get_size(table) - 1)) {
        merge_adjacent(table, idx);
        fix_overlap(table, idx);
    }
}
