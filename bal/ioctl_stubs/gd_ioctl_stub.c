/* Copyright © 2013-2014, Owen Shepherd
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
#include <gd_bal.h>

#ifdef __i386__
__asm(
"gd_ioctl:\n"
".global gd_ioctl\n"
"	mov 4(%esp), %eax\n"
"   jmp *(%eax)\n"
);

#elif defined(__amd64__)
__asm(
".global gd_ioctl\n"
"gd_ioctl:\n"
"	jmp *(%rsi)\n"
);

#elif defined(__arm__)

__asm(
".global gd_ioctl\n"
"gd_ioctl:\n"
"	ldr pc, [r0]\n"
);

#else
int gd_ioctl(gd_device_t dev, unsigned num, ...)
{
	va_list ap;
	va_start(ap, num);
	int rv = dev->ioctl(dev, GD_FORWARD_IOCTL, num, &ap);
	va_end(ap);
	return rv;
}
#endif