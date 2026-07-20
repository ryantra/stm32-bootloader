#ifndef BL_UART_H_
#define BL_UART_H_

#include <stdint.h>

/* Minimal polling USART2 driver for the NUCLEO-F411RE.
 * USART2 is wired to the on-board ST-LINK virtual COM port (PA2=TX, PA3=RX),
 * so no extra hardware is needed to talk to the bootloader. 115200 8N1. */
void    bl_uart_init(void);
void    bl_uart_putc(uint8_t c);
uint8_t bl_uart_getc(void);
int     bl_uart_has_byte(void);   /* non-zero if a byte is waiting */

#endif /* BL_UART_H_ */
