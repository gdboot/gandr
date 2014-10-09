#include <bal/misc.h>
#include <stdio.h>
#include <stdarg.h>

void panic(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "PANIC! \n");
    vfprintf(stderr, fmt, ap);

    fprintf(stderr, "\n\nHalted.\n");
    va_end(ap);

    for(;;);
}
