#include "kickstart.h"
#include <stdint.h>
#include <string.h>

/* semihosting file I/O */

int io_open(const char *filename)
{
    uintptr_t params[3];

    params[0] = (uintptr_t) filename;
    params[1] = 1;
    params[2] = (uintptr_t) strlen(filename);
    return shcall(SH_OPEN, params);
}

unsigned    io_len(int fd)
{
    uintptr_t params[1];
    params[0] = fd;
    return shcall(SH_FLEN, params);
}

int         io_seek(int fd, unsigned offset)
{
    uintptr_t params[2];
    params[0] = fd;
    params[1] = offset;
    return !(shcall(SH_SEEK, params) == offset);
}

int         io_read(int fd, void *buf, unsigned len)
{
    uintptr_t params[3];
    params[0] = fd;
    params[1] = (uintptr_t) buf;
    params[2] = len;

    /* WTF return values here! */
    int res = shcall(SH_READ, params);
    if (res == 0)
        return len;
    else if (res == (int) len)
        return -1;
    else return res;
}

void        io_close(int fd)
{
    uintptr_t params[1];
    params[0] = fd;
    shcall(SH_CLOSE, params);
}
