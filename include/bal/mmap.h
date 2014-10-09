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

#ifndef BAL_MMAP_H
#define BAL_MMAP_H
#include <gd_bal.h>

/*! Add an entry to the memory map */
void mmap_add_entry(gd_memory_map_entry entry);

/*! Gets the memory map
 *
 * \param tab         destination table
 * \param nentries    number of spaces in table
 * \param *needed     number of entries in whole memory map
 * \param *key        key to pass to gd_exit_boot_services
 */
void mmap_get(
    gd_memory_map_entry *tab,
    size_t nentries,
    size_t *needed,
    size_t *key);

#endif
