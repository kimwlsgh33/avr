#ifndef __UART_H__
#define __UART_H__
/*
 * uart interface.
 *
 * @author gcode@FHT
 *
 * 2024/10/15. multiple BAUD rate support,
 *        F_CPU definiation "MUST" be supplied.
 * 2014/10/13. putter/getter interface added.
 *        uartX_init() method added.
 * */

#if defined(__GNUC__)
#include <avr/interrupt.h>
#include <avr/io.h>
#else
#pragma error "ONLY GCC supported"
#endif

#define F_CPU 16000000
#ifndef F_CPU
#pragma error "F_CPU definition should be supplied"
#endif

#define BAUD_9600 9600L
#define BAUD_14400 14400L
#define BAUD_19200 19200L
#define BAUD_28800 28800L
#define BAUD_38400 38400L
#define BAUD_57600 57600L
#define BAUD_76800 76800L
#define BAUD_115200 115200L
#define BAUD_230400 230400L
#define BAUD_250000 250000L

/*
 * command writter (via uart) setting
 *
 *  int uart_putchar0(uint8 *c);
 * */
typedef int (*uart_getter)(uint8_t *c);
typedef int (*uart_putter)(uint8_t c);

int init_uart(uint32_t baud);
int getchar_uart(uint8_t *c);
int putchar_uart(uint8_t c);

#endif
