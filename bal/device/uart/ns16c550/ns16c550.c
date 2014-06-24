#include <uart/ns16c550.h>

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

    // Wait until there is FIFO space
    while(dev->fifo_space == 0) {
        // Is FIFO empty?
        if(rdreg(dev, UART_LSR) & LSR_TX_FIFO_EMPTY) {
            // It is - reset the FIFO space variable
            dev->fifo_space = dev->fifo_size;
        }
    }

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

int ns16c550_ioctl(ns16c550_dev *dev, gd_ioctl_t cmd, va_list ap)
{
    switch(cmd) {
        case gd_ioctl_write: {
            char const *buf = va_arg(void const*, ap);
            size_t nbytes   = va_arg(size_t, ap);
            return write(dev, buf, nbytes);
        }

        case gd_ioctl_read: {
            char *buf     = va_arg(void *, ap);
            size_t nbytes = va_arg(size_t, ap);
            return read(dev, buf, nbytes);
        }

        default:
            return uart_base_ioctl(&dev->ioctl, cmd, ap);
    }

    }
}
