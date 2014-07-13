/* Copyright © 2014, Owen Shepherd
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

#include <bal/bios_console.h>
#include <bal/bios_services.h>

static int bios_console_write(struct bios_console_dev *dev, const void *buf, size_t sz)
{
    const unsigned char *cbuf = buf;

    struct bios_registers regs = { 0 };

    for(size_t i = 0; i < sz; i++) {
        unsigned char c = cbuf[i];

        if(c == '\n') {
            // BIOS is a CRLF platform
            regs.ebx = dev->cur_page << 8;
            regs.eax = 0x0E00 | '\r';
            bios_int_call(0x10, &regs);
        }

        regs.eax = 0x0E00 | c;
        regs.ebx = dev->cur_page << 8;
        bios_int_call(0x10, &regs);
    }

    return 0;
}

GD_BEGIN_IOCTL_MAP(struct bios_console_dev *, bios_console_ioctl)
    GD_MAP_WRITE_IOCTL(bios_console_write)
GD_END_IOCTL_MAP()

void bios_console_init(struct bios_console_dev *dev)
{
    dev->dev.ioctl = bios_console_ioctl;

    struct bios_registers regs = { 0 };
    regs.eax = 0x0F00;
    bios_int_call(0x10, &regs);
    dev->width    = (regs.eax >> 8) & 0xFF;
    dev->cur_page = (regs.ebx >> 8) & 0xFF;
}