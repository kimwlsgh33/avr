#ifndef __UARTT1_H__
#define __UARTT1_H__
/*
 * HACK: AT128 uart interface.
 *
 * @version 1.0
 *   - WIN32 simulation codes are removed. (see the uart-sim.c/h)
 *
 * 2024/10/15. multiple BAUD rate support,
 *        F_CPU definication "MUST" be supplied.
 * 2014/10/13. putter/getter interface added.
 *        uartX_init() method added.
 * */
#include "uart.h"

int init_uart1(uint32_t baud);
int getchar_uart1(uint8_t *c);
int putchar_uart1(uint8_t c);
#endif
