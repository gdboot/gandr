#include <bal/device/uart/ns16c550.h>

ns16c550_dev uart0 = {
	.base      = (void*) 0x44E09000,
	.fifo_size = 64,
	.reg_width = 32
};

void puts(const char* str)
{
	while(*str) ns16c550_txbyte(&uart0, *str++);
}

void main(void)
{
	ns16c550_init(&uart0);
	for(;;) {
		puts("Hello, world!\r\n");
	}
}