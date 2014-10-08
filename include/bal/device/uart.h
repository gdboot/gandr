#ifndef GD_BAL_UART_H
#define GD_BAL_UART_H

enum gd_uart_parity {
	GD_UART_NO_PARITY 	= 0,
	GD_UART_ODD_PARITY 	= 1,
	GD_UART_EVEN_PARITY = 2,
};

enum gd_uart_flow_control {
	//! Serial port operating without flow control
	GD_UART_NO_FLOW_CONTROL = 0,
	//! RTS, CTS flow control (asymmetric).
	GD_UART_RTS_CTS_FLOW_CONTROL,
	//! RTR, CTS flow control (symmetric)
	GD_UART_RTR_CTS_FLOW_CONTROL
};

#define GD_FLOW_CONTROL_BIT(bit) (1U << ((unsigned)(bit)))

struct gd_uart_capabilities {
	unsigned	fifo_bytes;
	unsigned	max_baud;
	unsigned	flow_control_support;
};

struct gd_uart_config {
	unsigned 					baud;
	unsigned char 				bits;
	enum gd_uart_parity 		parity;
	unsigned char 				stop_bits;
	enum gd_uart_flow_control   flow_control;
};

int gd_uart_base_ioctl(gd_device_t dev, unsigned, ...);

#endif
