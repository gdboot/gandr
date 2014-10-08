/* Copyright © 2014, Owen Shepherd
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
#include <bal/device/uart/arm_pl011.h>
#include <bal/device/uart.h>
#include <errno.h>

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
enum uart_flag_bits {
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

/* UART_CR */
enum uart_cr_bits {
    UART_CR_UARTEN_BIT     = (1 << 0),
    UART_CR_SIREN_BIT      = (1 << 1),
    UART_CR_SIRLP_BIT      = (1 << 2),
    UART_CR_LBE_BIT        = (1 << 7),
    UART_CR_TXE_BIT        = (1 << 8),
    UART_CR_RXE_BIT        = (1 << 9),
    UART_CR_DTR_BIT        = (1 << 10),
    UART_CR_RTS_BIT        = (1 << 11),
    UART_CR_OUT1_BIT       = (1 << 12),
    UART_CR_OUT2_BIT       = (1 << 13),
    UART_CR_RTSEn_BIT      = (1 << 14),
    UART_CR_CTSEn_BIT      = (1 << 15),
};

/* UART_LCR_H */
enum uart_lcrh_bits {
    UART_LCR_H_BRK_BIT     = (1 << 0),
    UART_LCR_H_PEN_BIT     = (1 << 1),
    UART_LCR_H_EPS_BIT     = (1 << 2),
    UART_LCR_H_STP2_BIT    = (1 << 3),
    UART_LCR_H_FEN_BIT     = (1 << 4),
    UART_LCR_H_WLEN_BITS   = (3 << 5),
    UART_LCR_H_WLEN_SHIFT  = 5,
    UART_LCR_H_SPS_BIT     = (1 << 7),
};

static int pl011_write(arm_pl011_dev *dev, const char *buf, size_t nbytes, size_t *written)
{
    while (gio_read32_index(dev->base, UART_FR) & UART_FR_TXFF_BIT);

    *written = 0;
    for (size_t i = 0; i < nbytes; i++) {
        if (gio_read32_index(dev->base, UART_FR) & UART_FR_TXFF_BIT)
            break;
        gio_write32_index(dev->base, UART_DR, buf[i]);
        (*written)++;
    }

    return 0;
}

static int pl011_read(arm_pl011_dev *dev, char *buf, size_t nbytes, size_t *read)
{
    while (gio_read32_index(dev->base, UART_FR) & UART_FR_RXFE_BIT);

    *read = 0;
    for (size_t i = 0; i < nbytes; i++) {
        if (gio_read32_index(dev->base, UART_FR) & UART_FR_RXFE_BIT)
            break;
        buf[i] = gio_read32_index(dev->base, UART_DR);
        (*read)++;
    }

    return 0;
}

static int pl011_set_config(
    arm_pl011_dev *dev,
    const struct gd_uart_config *cfg)
{
    /* BRG is 16.6 fixed point register. Calculate and correctly round that */
    uint32_t brg = ((dev->brg_clock) / (cfg->baud * 16 * 128) + 1)  / 2;

    uint32_t cr = UART_CR_UARTEN_BIT | UART_CR_RXE_BIT | UART_CR_TXE_BIT;
    uint32_t lcrh = UART_LCR_H_FEN_BIT;
    switch (cfg->bits) {
        case 5: lcrh |= 0 << UART_LCR_H_WLEN_SHIFT; break;
        case 6: lcrh |= 1 << UART_LCR_H_WLEN_SHIFT; break;
        case 7: lcrh |= 2 << UART_LCR_H_WLEN_SHIFT; break;
        case 8: lcrh |= 3 << UART_LCR_H_WLEN_SHIFT; break;
        default: return EINVAL;
    }

    switch (cfg->parity) {
        case GD_UART_NO_PARITY:   break;
        case GD_UART_ODD_PARITY:  lcrh |= UART_LCR_H_PEN_BIT; break;
        case GD_UART_EVEN_PARITY: lcrh |= UART_LCR_H_PEN_BIT | UART_LCR_H_EPS_BIT; break;
    }

    switch (cfg->stop_bits) {
        case 1: break;
        case 2: lcrh |= UART_LCR_H_STP2_BIT; break;
        default: return EINVAL;
    }

    switch (cfg->flow_control) {
        case GD_UART_NO_FLOW_CONTROL:
            break;

        case GD_UART_RTS_CTS_FLOW_CONTROL:
            cr |= UART_CR_CTSEn_BIT | UART_CR_RTS_BIT;
            break;

        case GD_UART_RTR_CTS_FLOW_CONTROL:
            cr |= UART_CR_CTSEn_BIT | UART_CR_RTSEn_BIT;
            break;
    }

    /* disable device */
    gio_write32_index(dev->base, UART_CR, 0);

    /* reinit */
    gio_write32_index(dev->base, UART_IBRD, brg >> 6);
    gio_write32_index(dev->base, UART_FBRD, brg & 0x3F);
    gio_write32_index(dev->base, UART_LCR_H, lcrh);
    gio_write32_index(dev->base, UART_CR, cr);

    return 0;
}

GD_BEGIN_IOCTL_MAP(arm_pl011_dev *, arm_pl011_ioctl)
    GD_MAP_WRITE_IOCTL(pl011_write)
    GD_MAP_READ_IOCTL(pl011_read)
    GD_MAP_UART_SET_CONFIG_IOCTL(pl011_set_config)
GD_END_IOCTL_MAP()


void arm_pl011_init(arm_pl011_dev *dev)
{
    dev->dev.ioctl = arm_pl011_ioctl;

    /* Make sure DMA, interrupts disabled */
    gio_write32_index(dev->base, UART_DMACR, 0);
    gio_write32_index(dev->base, UART_IMSC, 0x7FF);

    uint32_t cr = gio_read32_index(dev->base, UART_CR);
    if ((cr & UART_CR_UARTEN_BIT) == 0) {
        struct gd_uart_config conf = {
            .baud   = 115200,
            .bits   = 8,
            .parity = GD_UART_NO_PARITY,
            .stop_bits = 1,
            .flow_control = GD_UART_NO_FLOW_CONTROL
        };

        pl011_set_config(dev, &conf);
    }
}
