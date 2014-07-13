/* Copyright © 2013, Owen Shepherd
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
#include <bal/device/uart/ns16c550.h>

//! NS16C550 Registers
enum ns16c550_reg {
    UART_DATA = 0,  //!< Dtata in/out register
    UART_IER  = 1,  //!< Interrupt Enable Register
    
    // When DLAB = 1
    UART_DIVISOR_LSB = 0,
    UART_DIVISOR_MSB = 1,

    UART_IIR = 2, //!< Interrupt Identification Register (RO)
    UART_FCR = 2, //!< FIFO Control Register (WO)
    UART_LCR = 3, //!< Line Control Register
    UART_MCR = 4, //!< Modem Control Register
    UART_LSR = 5, //!< Line Status Register
    UART_MSR = 6, //!< Modem Status Register
};

enum ns16c550_lsr_bit {
    //! RX fifo empty
    LSR_RX_FIFO_EMPTY   = (1 << 0),
    //! RX overrun error
    LSR_RX_OVERRUN_ERR  = (1 << 1),
    //! RX parity error
    LSR_RX_PARITY_ERR   = (1 << 2),
    //! RX framing error
    LSR_RX_FRAMING_ERR  = (1 << 3),
    //! RX has break condition
    LSR_RX_BREAK        = (1 << 4),
    //! TX fifo empty
    LSR_TX_FIFO_EMPTY   = (1 << 5),
    //! TX shift register empty
    LSR_TX_SR_EMPTY     = (1 << 6),
    //! RX FIFO contains error
    LSR_RX_FIFO_ERR     = (1 << 7),
};

enum ns16c550_fcr_bit {
    FCR_FIFO_EN         = (1 << 0),
    FCR_RX_FIFO_CLEAR   = (1 << 1),
    FCR_TX_FIFO_CLEAR   = (1 << 2),
    FCR_DMA_MODE        = (1 << 3),
};

static void wrreg(ns16c550_dev *dev, enum ns16c550_reg reg, unsigned char b)
{
    gio_write_index(dev->reg_width, dev->base, reg, b);
}

static unsigned char rdreg(ns16c550_dev *dev, enum ns16c550_reg reg)
{
    return gio_read_index(dev->reg_width, dev->base, reg);
}

void ns16c550_init(ns16c550_dev *dev)
{
    // TODO: initialize properly =)
    wrreg(dev, UART_FCR, FCR_FIFO_EN | FCR_TX_FIFO_CLEAR | FCR_RX_FIFO_CLEAR);
}

static int write(ns16c550_dev *dev, const char *buf, size_t nbytes)
{
    size_t i = 0;

    // Check FIFO level
    do {
        if(rdreg(dev, UART_LSR) & LSR_TX_SR_EMPTY) {
            // It is - reset the FIFO space variable
            dev->fifo_space = dev->fifo_size;
        }
    } while(dev->fifo_space == 0);

    // ..then write as much as possible
    while(i < nbytes && dev->fifo_space) {
        wrreg(dev, UART_DATA, buf[i++]);
        dev->fifo_space--;
    }

    return i;
}

static int read(ns16c550_dev *dev, char *buf, size_t nbytes)
{
    size_t i = 0;

    // Wait until there is something in the FIFO
    while((rdreg(dev, UART_LSR) & LSR_RX_FIFO_EMPTY) == 0);

    // Read as much as we can in one go
    while(i < nbytes && (rdreg(dev, UART_LSR) & LSR_RX_FIFO_EMPTY) != 0) {
        buf[i++] = rdreg(dev, UART_DATA);
    }
    return i;
}

GD_BEGIN_IOCTL_MAP(ns16c550_dev *, ns16c550_ioctl)
    GD_MAP_WRITE_IOCTL(write)
    GD_MAP_READ_IOCTL(read)
GD_END_IOCTL_MAP_FORWARD(uart_base_ioctl)
