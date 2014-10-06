/* Copyright © 2013-2014, Owen Shepherd
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