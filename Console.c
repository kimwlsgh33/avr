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
  if (c == '\n') {
    while (!STD_PUTCHAR('\r'))
      ;
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
void init_cons(uint32_t baud)
{
  STD_INIT_UART(baud);
  fdevopen(std_putchar, std_getchar);
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
