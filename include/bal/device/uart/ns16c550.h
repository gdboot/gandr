#ifndef NS16C550_H
#define NS16C550_H
#include <stdint.h>
#include <bal/gio.h>

typedef struct {
	//! Base address
	gio_addr base;
	//! FIFO size
	uint16_t fifo_size;
	//! Known available FIFO space
	uint16_t fifo_space;

	//! Register width
	uint8_t reg_width;
} ns16c550_dev;

void ns16c550_init(ns16c550_dev *dev);
void ns16c550_txbyte(ns16c550_dev *dev, char c);
char ns16c550_rxbyte(ns16c550_dev *dev);

#endif
