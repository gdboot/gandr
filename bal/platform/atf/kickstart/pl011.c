#include <stdint.h>
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

void putc(char c)
{
    if (c == '\n') {
        while(pl011_regs[UART_FR] & UART_FR_TXFF_BIT);
        pl011_regs[UART_DR] = (unsigned char) '\r';
    }

    /* wait for FIFO space */
    while(pl011_regs[UART_FR] & UART_FR_TXFF_BIT);
    pl011_regs[UART_DR] = (unsigned char) c;
}

void putsn(const char *str)
{
    while(*str)
        putc(*str++);
}

void puts(const char *str)
{
    putsn(str);
    putc('\n');
}
