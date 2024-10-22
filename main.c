/*
 * main.c
 *
 * Created: 10/2/2024 4:20:55 PM
 *  Author: FHT
 */
#include "Console.h"
#include "timer.h"
#include "uart.h"
#include "uart1.h"
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
/* #include <xc.h> */

#define BAUD_RATE 115200L

#define TEST_GPIO 0
#define TEST_OPTO 0
#define TEST_RELAY 0
#define TEST_UART 1
#define TEST_CONSOLE 1
#define TEST_TIMER 1

void init_gpio();
void init_opto();
void init_relay();
int init_dcps();

void toggle_gpios();
void toggle_optos();
void toggle_relays();

void uart_transmit(unsigned char data, unsigned char port);
void uart_trans_string(const char *str, unsigned char port);
unsigned char uart_receive(unsigned char port);

static int tid_op;
static int tid_st; // automatic state send interval, when st_send_mode == 1

/*
 * DCPS: Data Copies Per Second
 * */
#define DCP_MAX_PACKET_LEN 128
static uint8_t rxBuff[DCP_MAX_PACKET_LEN];
static uint8_t rxLen;
#define DCP_GETCHAR(pc) getchar_uart1(pc)

int main(void)
{
  //==================================================
  // initialize
  //==================================================
  // GPIO
#if TEST_GPIO
  init_gpio();
#endif

  // Optocoupler
#if TEST_OPTO
  init_opto();
#endif

  // Relay
#if TEST_RELAY
  init_relay();
#endif

#if TEST_CONSOLE
  init_cons(BAUD_115200);
#endif

#if TEST_TIMER
  init_timer();
#endif

#if TEST_CONSOLE
  init_dcps();
#endif

  // RS232 UART
  // 103: 16MHz, 9600bps, U2Xn = 0
  // 16: 16MHz, 115200bps, U2Xn = 0
#if TEST_UART
  /* init_dcps(); */
#endif

  //==================================================
  // function
  //==================================================
  while (1) {
#if TEST_GPIO
    toggle_gpios();
#endif
#if TEST_OPTO
    toggle_optos();
#endif
#if TEST_RELAY
    toggle_relays();
#endif
#if TEST_TIMER
    asm("sei");
    tid_op = alloc_timer();
    timer_set(tid_op, 0);
    tid_st = alloc_timer();
#endif
#if TEST_CONSOLE
    printf("CONSOLE WORKS v1.0 \n");
#endif
#if TEST_UART
#endif
    _delay_ms(1000);
  }
}

//==================================================
// initialize
//==================================================
void init_gpio()
{
  PORTF &= ~(1 << PINF0 | 1 << PINF1 | 1 << PINF2 | 1 << PINF3 | 1 << PINF4 |
             1 << PINF5 | 1 << PINF6 | 1 << PINF7);
  DDRF |= (1 << PINF0 | 1 << PINF1 | 1 << PINF2 | 1 << PINF3 | 1 << PINF4 |
           1 << PINF5 | 1 << PINF6 | 1 << PINF7);
}

void init_opto()
{
  PORTA &= ~(1 << PINA4 | 1 << PINA5 | 1 << PINA6 | 1 << PINA7);
  DDRA |= (1 << PINA4 | 1 << PINA5 | 1 << PINA6 | 1 << PINA7);
}

void init_relay()
{
  PORTC &= ~(1 << PINC4 | 1 << PINC5 | 1 << PINC6 | 1 << PINC7);
  DDRC |= (1 << PINC4 | 1 << PINC5 | 1 << PINC6 | 1 << PINC7);
}

int init_dcps()
{
  init_uart1(BAUD_115200);
  return 0;
}

//==================================================
// function
//==================================================
void toggle_gpios()
{
  PORTF ^= (1 << PINF0 | 1 << PINF1 | 1 << PINF2 | 1 << PINF3 | 1 << PINF4 |
            1 << PINF5 | 1 << PINF6 | 1 << PINF7);
}

void toggle_optos()
{
  PORTA ^= (1 << PINA4 | 1 << PINA5 | 1 << PINA6 | 1 << PINA7);
}

void toggle_relays()
{
  PORTC ^= (1 << PINC4 | 1 << PINC5 | 1 << PINC6 | 1 << PINC7);
}

int dcps_get_packet(char *out_buff)
{
  char c;
  /* (DCP_GETCHAR((unsigned char *)&c) == 1) */
  // HACK: Why not use the same type?
  if (DCP_GETCHAR((unsigned char *)&c)) {
    // packet end
    if (c == '\r') {
      int len = rxLen;

      rxBuff[rxLen] = 0;
      memcpy(out_buff, rxBuff, rxLen + 1);
      rxLen = 0;

      return len;
    } else {
      // payloads
      if (rxLen < (DCP_MAX_PACKET_LEN - 1)) {
        rxBuff[rxLen++] = c;
      }

      return -1;
    }
  }

  return -1;
}
