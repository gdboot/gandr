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
#include <bal/fs/iso9660.h>
#include <stdlib.h>
#include <errno.h>

static int translate_status(l9660_status stat)
{
    switch (stat) {
        case L9660_OK:         return 0;
        case L9660_EIO:        return EIO;
        case L9660_EBADFS:     return EIO;
        case L9660_ENOENT:     return ENOENT;
        case L9660_ENOTFILE:   return EISDIR;
        case L9660_ENOTDIR:    return ENOTDIR;
        default:               return EINVAL;
    }
}

typedef struct {
    struct gd_device dev;
    l9660_file impl;
} iso9660_file;

static int file_read(iso9660_file *f, void * buf, size_t nbytes, size_t * nbytesread)
{
    return translate_status(l9660_read(&f->impl, buf, nbytes, nbytesread));
}

static int file_pread(
    iso9660_file *f,
    void * buf,
    size_t nbytes,
    size_t * nbytesread,
    uint64_t offset)
{
    uint32_t old_offset = l9660_tell(&f->impl);
    l9660_status stat = l9660_read(&f->impl, buf, nbytes, nbytesread);
    l9660_status seek_stat = l9660_seek(&f->impl, SEEK_SET, old_offset);

    return translate_status(stat ? stat : seek_stat);
}

static int file_seek(iso9660_file *f, int64_t offset, int whence, int64_t * new_offset)
{
    if (offset > UINT32_MAX)
        return EINVAL;

    int rv = translate_status(l9660_seek(&f->impl, whence, offset));

    *new_offset = l9660_tell(&f->impl);
    return rv;
}

static int file_close(iso9660_file *f)
{
    free(f);
    return 0;
}

GD_BEGIN_IOCTL_MAP(iso9660_file *, file_ioctl)
    GD_MAP_READ_IOCTL(file_read)
    GD_MAP_PREAD_IOCTL(file_pread)
    GD_MAP_SEEK_IOCTL(file_seek)
    GD_MAP_CLOSE_IOCTL(file_close)
GD_END_IOCTL_MAP()

static int fs_openat(iso9660_fs *fs, gd_device_t *p_fd, const char * name)
{
    l9660_status stat;

    iso9660_file *file = malloc(sizeof(*file));
    if (!file)
        return ENOMEM;

    file->dev.ioctl = file_ioctl;

    stat = l9660_fs_open_root((l9660_dir*) &file->impl, &fs->impl);
    if (stat) goto cleanup;

    stat = l9660_openat(&file->impl, (l9660_dir*) &file->impl, name);
    if (stat) goto cleanup;

    *p_fd = &file->dev;
    return 0;

cleanup:
    free(file);
    return translate_status(stat);
}

GD_BEGIN_IOCTL_MAP(iso9660_fs *, fs_ioctl)
    GD_MAP_OPENAT_IOCTL(fs_openat)
GD_END_IOCTL_MAP()

static bool read_sector(l9660_fs *ifs, void *buf, uint32_t sector)
{
    iso9660_fs *fs = (iso9660_fs *)(((char*) ifs) - offsetof(iso9660_fs, impl));

    size_t read;
    if (gd_pread(fs->blockdev, buf, 2048, &read, sector * 2048))
        return false;

    if (read != 2048)
        return false;

    return true;
}

int iso9660fs_init(iso9660_fs *self, gd_device_t blockdev)
{
    self->dev.ioctl = fs_ioctl;
    self->blockdev  = blockdev;

    return translate_status(l9660_openfs(&self->impl, read_sector));
}
