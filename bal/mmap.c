/* Copyright © 2014, Shikhin Sethi
 * Copyright © 2014, Owen Shepherd
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

#include <bal/mmap.h>
#include <bal/misc.h>
#include <gd_syscall.h>
#include <gd_tree.h>
#include <string.h>
#include <errno.h>

typedef struct mmap_entry {
    RB_ENTRY(mmap_entry) rbnode;
    gd_memory_map_entry  entry;
} mmap_entry;

static int mmap_entry_cmp(const mmap_entry *l, const mmap_entry *r)
{
    return l->entry.physical_start < r->entry.physical_start ? -1 :
           l->entry.physical_start > r->entry.physical_start ? +1 : 0;
}

static RB_HEAD(mmap_tree, mmap_entry) mmap = RB_INITIALIZER(&mmap);
RB_PROTOTYPE_STATIC(mmap_tree, mmap_entry, rbnode, mmap_entry_cmp)

/* When we free mmap entries, we place them in a linked list built out of their
 * ent->rbnode.rbe_left fields
 */
static mmap_entry *last_freed = NULL;

/* We pre-allocate 32 mmap entries as empirically sufficient to hold a system
 * memory map. If it isn't, then we will scavenge from that memory map (if you
 * don't have any memory in the first 32 entries we find, WTF is with your
 * system).
 */
#define MMAP_STATIC_SLOTS 32
static mmap_entry static_slots[MMAP_STATIC_SLOTS];
static mmap_entry *alloc_next  = static_slots;
static size_t      allocatable = MMAP_STATIC_SLOTS;
/* incremented each time we update the memory map */
static size_t      mmap_key     = 0;

static gd_memory_type overlap_precedence[] = {
    /*! The overlapping region is promoted to the one that comes earlier. */
    gd_unusable_memory, gd_pal_code, gd_mmio_port_space, gd_mmio,
    gd_acpi_memory_nvs, gd_runtime_services_code, gd_runtime_services_data,
    gd_acpi_reclaim_memory, gd_loader_code, gd_loader_data, gd_boot_services_code,
    gd_boot_services_data, gd_conventional_memory, gd_reserved_memory_type
};


static void mmap_free_entry(mmap_entry *ent)
{
    ent->rbnode.rbe_left  = last_freed;
    ent->rbnode.rbe_right = ent->rbnode.rbe_parent = NULL;
    last_freed = ent;
}

static void merge_adjacent(mmap_entry *middle)
{
    mmap_entry *prev = RB_PREV(mmap_tree, &mmap, middle);
    mmap_entry *next = RB_NEXT(mmap_tree, &mmap, middle);

    /* TODO: attributes */
    if (prev && prev->entry.type == middle->entry.type
            && prev->entry.physical_start + prev->entry.size
                == middle->entry.physical_start) {
        prev->entry.size += middle->entry.size;
        RB_REMOVE(mmap_tree, &mmap, middle);
        mmap_free_entry(middle);
        ++mmap_key;
        middle = prev;
    }

    if (next && middle->entry.type == next->entry.type
            && middle->entry.physical_start + middle->entry.size
                == next->entry.physical_start) {
        middle->entry.size += next->entry.size;
        RB_REMOVE(mmap_tree, &mmap, next);
        mmap_free_entry(next);
        ++mmap_key;
    }
}

static mmap_entry *mmap_alloc_entry(void)
{
    mmap_entry *mme = NULL;
    if (last_freed) {
        mme = last_freed;
        last_freed = last_freed->rbnode.rbe_left;
        return mme;
    } else {
        if (!allocatable) {
            RB_FOREACH (mme, mmap_tree, &mmap) {
                if (mme->entry.physical_start + 4096 > UINTPTR_MAX)
                    break;

                if (mme->entry.type == gd_conventional_memory) {
                    mmap_entry *neighbour;
                    if (mme->entry.size == 4096) {
                        alloc_next  = (mmap_entry*)(uintptr_t) mme->entry.physical_start;
                        allocatable = 4096 / sizeof(*mme);

                        mme->entry.type = gd_loader_data;
                        merge_adjacent(mme);
                    } else if ((neighbour = RB_PREV(mmap_tree, &mmap, mme))
                            && neighbour->entry.type == gd_loader_data
                            && (neighbour->entry.physical_start + neighbour->entry.size)
                                == mme->entry.physical_start) {
                        // slice out first 4kiB

                        alloc_next  = (mmap_entry*)(uintptr_t) mme->entry.physical_start;
                        allocatable = 4096 / sizeof(*mme);

                        neighbour->entry.size += 4096;
                        mme->entry.size -= 4096;
                        mme->entry.physical_start += 4096;
                    } else if ((neighbour = RB_NEXT(mmap_tree, &mmap, mme))
                            && neighbour->entry.type == gd_loader_data
                            && (mme->entry.physical_start + mme->entry.size)
                                == neighbour->entry.physical_start) {

                        if (mme->entry.physical_start + mme->entry.size > UINTPTR_MAX)
                            break;

                        // slice out last 4kB
                        neighbour->entry.size += 4096;
                        neighbour->entry.physical_start -= 4096;
                        mme->entry.size -= 4096;

                        alloc_next  = (mmap_entry*)(uintptr_t) neighbour->entry.physical_start;
                        allocatable = 4096 / sizeof(*mme);
                    } else {
                        // neither neigbour is appropriate
                        alloc_next  = (mmap_entry*)(uintptr_t) mme->entry.physical_start;
                        allocatable = 4096 / sizeof(*mme) - 1;
                        alloc_next->entry.type = gd_loader_data;
                        alloc_next->entry.physical_start = mme->entry.physical_start;
                        alloc_next->entry.size = 4096;
                        alloc_next->entry.attributes = 0;

                        mme->entry.physical_start += 4096;
                        mme->entry.size -= 4096;

                        RB_INSERT(mmap_tree, &mmap, alloc_next);
                        alloc_next += 1;
                    }
                    ++mmap_key;
                    break;
                }
            }

            if (!allocatable) {
                // Didn't manage to grab any space. Panic!
                panic("Out of memory");
            }
        }
        mme = alloc_next;
        alloc_next += 1;
        allocatable--;
        return mme;
    }
}

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

/* Fixes overlap bewteen \p first and the following node */
/* Returns non-zero value if removes first from mmap. */
static int fix_overlap(mmap_entry *first)
{
    if (!first) return 0;
    mmap_entry *second = RB_NEXT(mmap_tree, &mmap, first);
    gd_memory_map_entry new_entry = { 0 };
    if (!second) return 0;

    if ((first->entry.physical_start + first->entry.size)
            <= second->entry.physical_start)
        return 0;

    gd_memory_type type = higher_precedence(first->entry.type, second->entry.type);
    uint64_t first_physical_end = first->entry.physical_start + first->entry.size;
    uint64_t second_physical_end = second->entry.physical_start + second->entry.size;

    if (first_physical_end < second_physical_end) {
        // Partition
        type = higher_precedence(first->entry.type, second->entry.type);

        if (type == first->entry.type) {
            second->entry.size = second_physical_end - first_physical_end;
            second->entry.physical_start = second_physical_end - second->entry.size;
            return 0;
        } else if (type == second->entry.type) {
            first->entry.size = first->entry.physical_start - second->entry.physical_start;
            return 0;
        }

        new_entry.physical_start = first_physical_end;
        new_entry.size = second_physical_end - first_physical_end;
        new_entry.type = type;
        new_entry.attributes = second->entry.attributes;
    } else if (first_physical_end > second_physical_end) {
        // Second entry is a subset

        if (type == first->entry.type) {
            // Redundant second entry.
            RB_REMOVE(mmap_tree, &mmap, second);
            mmap_free_entry(second);

            return fix_overlap(first);
        }

        new_entry.physical_start = second_physical_end;
        new_entry.size = first_physical_end - second_physical_end;
        new_entry.type = first->entry.type;
        new_entry.attributes = first->entry.attributes;
    } else {
        // Second entry is end of first
        second->entry.type = type;
        if (!(first->entry.size = second->entry.physical_start - first->entry.physical_start)) {
            RB_REMOVE(mmap_tree, &mmap, first); mmap_free_entry(first);
            return 1;
        }
        return 0;
    }

    second->entry.attributes = first->entry.attributes | second->entry.attributes;
    second->entry.type = type;
    second->entry.size = new_entry.physical_start - second->entry.physical_start;

    if (!(first->entry.size = second->entry.physical_start - first->entry.physical_start)) {
        RB_REMOVE(mmap_tree, &mmap, first); mmap_free_entry(first);
        mmap_add_entry(new_entry); return 1;
    }

    mmap_add_entry(new_entry);
    return 0;
}

int gd_alloc_pages(gd_memory_type type, void **presult, size_t count)
{
    if (!count)
        return 0;

    mmap_entry *mme = NULL;
    RB_FOREACH (mme, mmap_tree, &mmap) {
        if (mme->entry.physical_start + count * 4096 > UINTPTR_MAX)
            break;

        if (mme->entry.type == gd_conventional_memory
            && mme->entry.size >= count * 4096) {
            mmap_entry *neighbour;

            if (mme->entry.size == count * 4096) {
                mme->entry.type = type;
                merge_adjacent(mme);
            } else if ((neighbour = RB_PREV(mmap_tree, &mmap, mme))
                    && neighbour->entry.type == type
                    && (neighbour->entry.physical_start + neighbour->entry.size)
                        == mme->entry.physical_start) {
                // slice out first `count` pages.

                *presult  = (void*)(uintptr_t) mme->entry.physical_start;

                neighbour->entry.size += count * 4096;
                mme->entry.size -= count * 4096;
                mme->entry.physical_start += count * 4096;
            } else if ((neighbour = RB_NEXT(mmap_tree, &mmap, mme))
                    && neighbour->entry.type == type
                    && (mme->entry.physical_start + mme->entry.size)
                        == neighbour->entry.physical_start) {

                if (mme->entry.physical_start + mme->entry.size > UINTPTR_MAX)
                    break;

                // slice out last `count` pages.
                neighbour->entry.size += count * 4096;
                neighbour->entry.physical_start -= count * 4096;
                mme->entry.size -= count * 4096;

                *presult  = (void*)(uintptr_t) neighbour->entry.physical_start;
            } else {
                // neither neigbour is appropriate
                *presult  = (void*)(uintptr_t) mme->entry.physical_start;

                mmap_entry *new_entry = mmap_alloc_entry();
                new_entry->entry.type = type;
                new_entry->entry.physical_start = mme->entry.physical_start;
                new_entry->entry.size = count * 4096;
                new_entry->entry.attributes = mme->entry.attributes;

                mme->entry.physical_start += count * 4096;
                mme->entry.size -= count * 4096;

                RB_INSERT(mmap_tree, &mmap, new_entry);
            }

            ++mmap_key;
            return 0;
        }
    }

    return ENOMEM;
}

int gd_free_pages(void *start_address, size_t count)
{
    /* Can free anything apart from unusable memory. */
    gd_memory_type free_overlap_precedence[] = {
        gd_unusable_memory, gd_conventional_memory, gd_pal_code, gd_mmio_port_space, gd_mmio,
        gd_acpi_memory_nvs, gd_runtime_services_code, gd_runtime_services_data,
        gd_acpi_reclaim_memory, gd_loader_code, gd_loader_data, gd_boot_services_code,
        gd_boot_services_data, gd_reserved_memory_type
    }, temp[sizeof overlap_precedence / sizeof (gd_memory_type)];

    gd_memory_map_entry free = {
        .physical_start = (uintptr_t) start_address,
        .size = count * 4096,
        .type = gd_conventional_memory
    };

    memcpy(&temp, &overlap_precedence, sizeof overlap_precedence);
    memcpy(&overlap_precedence, &free_overlap_precedence, sizeof free_overlap_precedence);
    mmap_add_entry(free);
    memcpy(&overlap_precedence, &temp, sizeof overlap_precedence);

    return 0;
}

void mmap_add_entry(gd_memory_map_entry entry)
{
    if (!entry.size)
        return;

    if (entry.physical_start & 0xFFF) {
        gd_memory_map_entry unusable = {
            .physical_start = entry.physical_start & ~0xFFF,
            .size = 0x1000,
            .type = gd_unusable_memory
        };
        mmap_add_entry(unusable);
        if (entry.size > (0x1000 - (entry.physical_start & 0xFFF)))
            entry.size -= 0x1000 - (entry.physical_start & 0xFFF);
        else return;
        entry.physical_start = unusable.physical_start + unusable.size;
    }

    if (entry.size & 0xFFF) {
        gd_memory_map_entry unusable = {
            .physical_start = (entry.physical_start + entry.size) & ~0xFFF,
            .size = 0x1000,
            .type = gd_unusable_memory
        };
        mmap_add_entry(unusable);
        if (!(entry.size &= ~0xFFF))
            return;
    }

    mmap_entry *newent = mmap_alloc_entry();
    memcpy(&newent->entry, &entry, sizeof entry);
    RB_INSERT(mmap_tree, &mmap, newent);

    if (!fix_overlap(newent)) {
        fix_overlap(RB_PREV(mmap_tree, &mmap, newent));
        merge_adjacent(newent);
    }

    ++mmap_key;
}

void mmap_clean(void)
{
    mmap_entry *mme, *next;
    for (mme = RB_MIN(mmap_tree, &mmap); mme; mme = next) {
        next = RB_NEXT(mmap_tree, &mmap, mme);
        RB_REMOVE(mmap_tree, &mmap, mme);
    }

    alloc_next  = static_slots;
    allocatable = MMAP_STATIC_SLOTS;
    last_freed  = NULL;
}

void mmap_get(
    gd_memory_map_entry *tab,
    size_t nentries,
    size_t *needed,
    size_t *key)
{
    size_t i = 0;
    mmap_entry *ent;
    RB_FOREACH (ent, mmap_tree, &mmap) {
        if (i < nentries) {
            memcpy(&tab[i], &ent->entry, sizeof *tab);
        }
        i++;
    }
    *needed = i;
    *key = mmap_key;
}

RB_GENERATE_STATIC(mmap_tree, mmap_entry, rbnode, mmap_entry_cmp)
