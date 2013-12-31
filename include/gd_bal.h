#ifndef GD_BAL_H
#define GD_BAL_H
#include <stdarg.h>

#define GD_CONCAT_(a, b) a##b
#define GD_CONCAT(a, b) GD_CONCAT_(a, b)

#define GD_UNPACK_(...) __VA_ARGS__
#define GD_UNPACK(x)    GD_UNPACK_ x

typedef struct gd_device {
    int (*ioctl)(struct gd_device *, unsigned, ...);
} *gd_device_t;

#endif