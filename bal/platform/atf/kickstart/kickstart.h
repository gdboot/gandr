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

#ifndef KICKSTART_H
#define KICKSTART_H

#define ESC_NORMAL "\033[0m"
#define ESC_BOLD   "\033[1m"

enum semihost_call {
    SH_OPEN     = 0x01,
    SH_CLOSE    = 0x02,
    SH_WRITEC   = 0x03,
    SH_WRITE0   = 0x04,
    SH_WRITE    = 0x05,
    SH_READ     = 0x06,
    SH_READC    = 0x07,
    SH_ISERROR  = 0x08,
    SH_ISTTY    = 0x09,
    SH_SEEK     = 0x0A,
    SH_FLEN     = 0x0C,
    SH_TMPNAM   = 0x0D,
    SH_REMOVE   = 0x0E,
    SH_RENAME   = 0x0F,
    SH_CLOCK    = 0x10,
    SH_TIME     = 0x11,
    SH_SYSTEM   = 0x12,
    SH_ERRNO    = 0x13,
};

/*! semihosting service call*/
int shcall(enum semihost_call, ...);

/*! uart functions */
void putc(char);
void puts(const char*);
void putsn(const char*);

/*! io functions */
int         io_open(const char *name);
unsigned    io_len(int fd);
int         io_seek(int fd, unsigned offset);
int         io_read(int fd, void *buf, unsigned len);
void        io_close(int fd);

void ks_main(void);
void *ks_load_dtb(void);
void *ks_load_bal(void);

#endif
