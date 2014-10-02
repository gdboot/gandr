#ifndef GD_BAL_FS_ISO9660_H
#define GD_BAL_FS_ISO9660_H
#include <stdint.h>
#include <gd_bal.h>
#include <lib9660.h>

typedef struct {
    struct gd_device dev;
    gd_device_t blockdev;
    l9660_fs impl;
} iso9660_fs;

int iso9660fs_init(iso9660_fs *self, gd_device_t blockdev);

#endif
