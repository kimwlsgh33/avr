#include "Console.h"
#include "uart.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if 0
#define STD_INIT_UART init_uart1
#define STD_GET_CHAR getchar_uart1
#define STD_PUT_CHAR putchar_uart1
#else
// NOTE: You don't need to know about what the standard output really is.
#define STD_INIT_UART init_uart
#define STD_GETCHAR getchar_uart
#define STD_PUTCHAR putchar_uart
#endif

// uart interface with pc
#define MAC_CMD_LEN 128
static char rxBUff[MAC_CMD_LEN];
static int rxLen = 0;

#ifdef USE_CONSOLE

static int std_putchar(char c, FILE *stream)
{
  // NOTE: repeat until '\r' if '\n' is found
  if (c == '\n') {
    STD_PUTCHAR('\r');
  }

  while (!STD_PUTCHAR(c))
    ;

  return 0;
}

static int std_getchar(FILE *stream)
{
  unsigned char c;
  while (STD_GETCHAR(&c) == 0) {
    asm("nop");
  }
  return c;
}

// Use uart0 as stdio
static FILE uart_stream =
    FDEV_SETUP_STREAM(std_putchar, std_getchar, _FDEV_SETUP_RW);
void init_cons(uint32_t baud)
{
  STD_INIT_UART(baud);
  /* fdevopen(std_putchar, std_getchar); */
  stdout = &uart_stream;
  stdin = &uart_stream;
}

int get_cmdline_cons(char *pOutBuff)
{
  uint8_t c;

  if (STD_GETCHAR(&c)) {
    if (c == '\r') {
      int len = rxLen;

      strcpy(pOutBuff, rxBUff);
    } else {
    }
  }
  return 0;
}
#endif
