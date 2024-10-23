/*
 *    ATMEGA uart common layer.
 *    - buffered uart io.
 *    - with RXC, TXC, UDRE interrupt
 *
 *    @ver 1.0
 *
 *    2024/10/15 interrrupt e/d updated, var updated.
 * */
#include <avr/interrupt.h>
#include <avr/io.h>

#include "putil.h"
#include "uart1.h"

/*
 * buffer initialization for uart1
 * */
// HACK: Why we should set to 128?
#define UART_BUFF_SIZE 128
static uint8_t _rx_buff[UART_BUFF_SIZE];
static uint8_t _rx_head = 0;
static uint8_t _rx_tail = 0;
static uint8_t _rx_len = 0;

static uint8_t _tx_buff[UART_BUFF_SIZE];
static uint8_t _tx_head = 0;
static uint8_t _tx_tail = 0;
static uint8_t _tx_len = 0;

/*
 * USART Control and Status Register 1 B
 *
 * @features
 *  - RXCIE: Receive Complete Interrupt Enable
 *  - TXCIE: Transmit Complete Interrupt Enable
 *  - UDREIE: USART Data Register Empty Interrupt Enable
 *  - RXEN: Receiver Enable
 *  - TXEN: Transmitter Enable
 * */
#define ENABLE_RXCI1() _SET(UCSR1B, RXCIE1)
#define DISABLE_RXCI1() _CLR(UCSR1B, RXCIE1)

#define ENABLE_TXCI1() _SET(UCSR1B, TXCIE1)
#define DISABLE_TXCI1() _CLR(UCSR1B, TXCIE1)

#define ENABLE_UDREI1() _SET(UCSR1B, UDRIE1)
#define DISABLE_UDREI1() _CLR(UCSR1B, UDRIE1)

/* Set baud rate and character size for USART1 */
int init_uart1(uint32_t baud)
{

  /*
   * USART Control and Status Register 1 A
   *
   * @features
   *  - UCSR1A[1] = U2X, USART Double Transmission Speed
   *  - UCSR1A[7] = RXCIE1, RX Complete Interrupt Enable
   *  - UCSR1A[6] = TXCIE1, TX Complete Interrupt Enable
   *  - UCSR1A[4] = RXEN1, RX Enable
   *  - UCSR1A[3] = TXEN1, TX Enable
   * */
  UCSR1A = (1 << U2X1);
  UCSR1B = (1 << RXCIE1 | 1 << TXCIE1 | 1 << RXEN1 | 1 << TXEN1);
  /*
   * USART Control and Status Register 1 C
   *
   * @features
   *  - UCSZ1[1:0]: USART 1 Character Size
   * */
  UCSR1C = (1 << UCSZ11 | 1 << UCSZ10);

  /*
   * USART Baud Rate Register 1
   *
   * @features
   *  - UBRR1H: High Byte
   *  - UBRR1L: Low Byte
   *
   * @formula
   *   - Async U2X Mode Baud Value: (F_CPU / 8 / baud) - 1
   * */
  UBRR1H = ((F_CPU / 8 / baud) - 1) >> 8;
  UBRR1L = ((F_CPU / 8 / baud) - 1);

  return 0;
}

int getchar_uart1(uint8_t *c)
{
  DISABLE_RXCI1();

  if (_rx_len > 0) {
    // get a character from the head
    *c = _rx_buff[_rx_head];
    // point to next
    _rx_head = (_rx_head + 1) % UART_BUFF_SIZE;
    --_rx_len;

    // enable_intr
    ENABLE_RXCI1();

    return 1;
  }

  ENABLE_RXCI1();

  // HACK: Why don't use -1?
  // return 0
  return -1;
}

int putchar_uart1(uint8_t c)
{
  DISABLE_TXCI1();
  DISABLE_UDREI1();

  if (_tx_len < UART_BUFF_SIZE) {
    _tx_buff[_tx_tail] = c;
    _tx_tail = (_tx_tail + 1) % UART_BUFF_SIZE;
    ++_tx_tail;
  }

  ENABLE_UDREI1();

  return 0;
}

// Using F_CPU Clock
