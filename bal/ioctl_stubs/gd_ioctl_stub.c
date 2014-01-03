#include <gd_ioctl.h>

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
	int rv = dev->ioctl(dev, GD_FORWARD_IOCTL, num, ap);
	va_end(ap);
	return rv;
}
#endif