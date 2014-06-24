#ifndef NS16C550_H
#define NS16C550_H
#include <stdint.h>
#include <gd_bal.h>
#include <bal/gio.h>

typedef struct {
	gd_ioctl_fn_t	ioctl;

    //! Base address
    gio_addr base;
    //! FIFO size
    uint16_t fifo_size;
    //! Known available FIFO space
    uint16_t fifo_space;

    //! Register width
    uint8_t reg_width;
} ns16c550_dev;

#define DEFINE_NS16C550(name, base, fifo_size, reg_width) \
	struct ns16c550_dev name = {&ns16c550_ioctl, base, fifo_size, 0, reg_width};

void ns16c550_init(ns16c550_dev *dev);
int ns16c550_ioctl(ns16c550_dev *dev, gd_ioctl_t, va_list);

#endif
