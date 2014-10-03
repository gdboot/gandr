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

#include <device/cdrom.h>
#include <bal/bios_services.h>
#include <errno.h>

static volatile struct {
    uint8_t size;
    uint8_t reserved;
    uint32_t blocks;
    uint16_t buffer_off, buffer_seg;
    uint64_t lba;
} __attribute__((packed)) lba_packet;

static int pread(cdrom_dev *dev, void *buf, size_t nbytes, size_t *nbytesread, uint64_t offset)
{
    *nbytesread = 0;

    if ((uintptr_t) buf > (0x10000 - 2048))
        return EINVAL;

    if (nbytes % 2048 || offset % 2048)
        return EINVAL;

    lba_packet.size = sizeof lba_packet;
    lba_packet.reserved = 0;
    lba_packet.blocks = (nbytes / 2048) > 0x7F ? 0x7F : nbytes / 2048;
    lba_packet.buffer_off = (uintptr_t) buf & 0x000F;
    lba_packet.buffer_seg = ((uintptr_t) buf & 0xFFFF0) >> 4;
    lba_packet.lba = offset / 2048;

    struct bios_registers regs = {
        .eax = 0x4200,
        .edx = dev->drive_number
    };
    regs.ds = ((uintptr_t) &lba_packet & 0xFFFF0) >> 4;
    regs.edi = (uintptr_t) &lba_packet & 0x000F;
    bios_int_call(0x13, &regs);

    if (regs.eflags & carry_flag || !lba_packet.blocks) {
        if (nbytes > 2048)
            return pread(dev, buf, 2048, nbytesread, offset);
        else return EIO;
    }

    return *nbytesread = lba_packet.blocks * 2048, 0;
}

GD_BEGIN_IOCTL_MAP(cdrom_dev *, cdrom_ioctl)
    GD_MAP_PREAD_IOCTL(pread)
GD_END_IOCTL_MAP()

void cdrom_init(cdrom_dev *dev, uint8_t drive_number)
{
    dev->dev.ioctl = cdrom_ioctl;
    dev->drive_number = drive_number;
}
