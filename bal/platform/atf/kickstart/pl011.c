#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include "kickstart.h"

enum reg {
    UART_DR = 0,
    UART_RSR,
    UART_ECR = UART_RSR,
    UART_RESV0, UART_RESV1, UART_RESV2, UART_RESV3,
    UART_FR,
    UART_RESV4,
    UART_ILPR,
    UART_IBRD,
    UART_FBRD,
    UART_LCR_H,
    UART_CR,
    UART_FLS,
    UART_IMSC,
    UART_RIS,
    UART_MIS,
    UART_ICR,
    UART_DMACR,
};

/* UART_FR */
enum flag_bits {
    UART_FR_CTS_BIT  = (1 << 0),
    UART_FR_DSR_BIT  = (1 << 1),
    UART_FR_DCD_BIT  = (1 << 2),
    UART_FR_BUSY_BIT = (1 << 3),
    UART_FR_RXFE_BIT = (1 << 4),
    UART_FR_TXFF_BIT = (1 << 5),
    UART_FR_RXFF_BIT = (1 << 6),
    UART_FR_TXFE_BIT = (1 << 7),
    UART_FR_RI_BIT   = (1 << 8),
};

extern volatile uint32_t pl011_regs[];

void uputc(char c)
{
    if (c == '\n') {
        while(pl011_regs[UART_FR] & UART_FR_TXFF_BIT);
        pl011_regs[UART_DR] = (unsigned char) '\r';
    }

    /* wait for FIFO space */
    while(pl011_regs[UART_FR] & UART_FR_TXFF_BIT);
    pl011_regs[UART_DR] = (unsigned char) c;
}

int puts(const char *str)
{
    while(*str)
        uputc(*str++);
    uputc('\n');
    return 0;
}

static size_t printfcb( void *p, const char *buf, size_t size )
{
    for (size_t i = 0; i < size; i++ )
        uputc(buf[i]);
    return size;
}

int printf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = _vcbprintf(0, printfcb, fmt, ap);
    va_end(ap);

    return n;
}
