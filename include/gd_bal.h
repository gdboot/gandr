/* Copyright © 2013-2014, Owen Shepherd & Shikhin Sethi
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

#ifndef GD_BAL_H
#define GD_BAL_H
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "gd_common.h"
#include "bal/gio.h"

#define GD_CONCAT_(a, b) a##b
#define GD_CONCAT(a, b) GD_CONCAT_(a, b)

#define GD_UNPACK_(...) __VA_ARGS__
#define GD_UNPACK(x)    GD_UNPACK_ x

typedef struct gd_device {
    int (*ioctl)(struct gd_device *, unsigned, ...);
} *gd_device_t;

#define GD_DEVICE union { \
        struct gd_device dev; \
        int (*ioctl)(struct gd_device *, unsigned, ...); \
    }

int (*gd_syscall_p)(unsigned, ...);
int gd_ioctl(gd_device_t, unsigned, ...);
int gd_syscall(unsigned, ...);

#include "gd_syscall.h"
#include "gd_ioctl.h"
#include "gd_ioctl_map.h"
#define GD_BEGIN_IOCTL_MAP(_type, _name)                                       \
int _name(gd_device_t dev_, unsigned ioctl, ...);                              \
int _name(gd_device_t dev_, unsigned ioctl, ...)                               \
{                                                                              \
    int rv = 0;                                                                \
    _type dev = (_type) dev_;                                                  \
    va_list ap, *pap = &ap;                                                    \
    va_start(ap, ioctl);                                                       \
postForward:                                                                   \
    switch(ioctl) {                                                            \
    case GD_FORWARD_IOCTL: {                                                   \
        ioctl = va_arg(*pap, unsigned);                                        \
        pap   = va_arg(*pap, va_list *);                                       \
        goto postForward;                                                      \
    }

#define GD_BEGIN_IOCTL_BASE_MAP(_type, _name)                                  \
int _name(gd_device_t dev_, unsigned ioctl, va_list *pap)                      \
{                                                                              \
    int rv = 0;                                                                \
    _type dev = (_type) dev_;                                                  \
postForward:                                                                   \
    switch(ioctl) {                                                            \
    case GD_FORWARD_IOCTL: {                                                   \
        ioctl = va_arg(*pap, unsigned);                                        \
        pap   = va_arg(*pap, va_list *);                                       \
        goto postForward;                                                      \
    }

#define GD_END_IOCTL_MAP()                                                     \
    default:                                                                   \
        /*errno = EINVAL;*/                                                    \
        rv = -1;                                                               \
    }                                                                          \
    va_end(ap);                                                                \
    return rv;                                                                 \
}

#define GD_END_IOCTL_BASE_MAP()                                                \
    default:                                                                   \
        /*errno = EINVAL;*/                                                    \
        rv = -1;                                                               \
    }                                                                          \
    return rv;                                                                 \
}


#define GD_END_IOCTL_MAP_FORWARD(_forwardTo)                                   \
    default:                                                                   \
        rv = (_forwardTo)(dev_, GD_FORWARD_IOCTL, ioctl, pap);                 \
    }                                                                          \
    va_end(ap);                                                                \
    return rv;                                                                 \
}

#define GD_END_IOCTL_MAP_FORWARD_BASE(_forwardTo)                              \
    default:                                                                   \
        rv = (_forwardTo)(dev_, ioctl, pap);                                   \
    }                                                                          \
    va_end(ap);                                                                \
    return rv;                                                                 \
}

#endif
