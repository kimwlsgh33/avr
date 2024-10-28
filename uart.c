/*
 * buffer initialization for default uart (USB to Serial)
 * */
// HACK: Why we should set to 128?
#include "uart.h"
#include "putil.h"
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <stdint.h>
/* #define BAUD 115200 */
/* #include <util/setbaud.h> */

#define USART_BUFF_SIZE 128
static uint8_t _rx_buff[USART_BUFF_SIZE];
static uint8_t _rx_head = 0;
static uint8_t _rx_tail = 0;
static uint8_t _rx_len = 0;

static uint8_t _tx_buff[USART_BUFF_SIZE];
static uint8_t _tx_head = 0;
static uint8_t _tx_tail = 0;
static uint8_t _tx_len = 0;

#ifdef UCSR0A
#define REG_UCSRA UCSR0A
#define REG_UCSRB UCSR0B
#define REG_UCSRC UCSR0C
#define REG_UBRRH UBRR0H
#define REG_UBRRL UBRR0L
#define REG_UDR UDR0
#define BIT_RXC RXC0
#define USART_RX_VECT USART0_RX_vect
#define USART_TX_VECT USART0_TX_vect
#define USART_UDRE_VECT USART0_UDRE_vect

// for uart0
#define DISABLE_RXCI() cbi(UCSR0B, RXCIE0)
#define ENABLE_RXCI() sbi(UCSR0B, RXCIE0)

#define DISABLE_TXCI() cbi(UCSR0B, TXCIE0)
#define ENABLE_TXCI() sbi(UCSR0B, TXCIE0)

#define DISABLE_UDREI() cbi(UCSR0B, UDRIE0)
#define ENABLE_UDREI() sbi(UCSR0B, UDRIE0)

#elif defined(UCSRA)

#define REG_UCSRA UCSRA
#define REG_UCSRB UCSRB
#define REG_UCSRC UCSRC
#define REG_UBRRH UBRRH
#define REG_UBRRL UBRRL
#define REG_UDR UDR
#define BIT_RXC RXC
#define UART_RX_VECT USART_RX_vect
#define UART_TX_VECT USART_TX_vect
#define UART_UDRE_VECT USART_UDRE_vect

#else
#pragma error "F_CPU definition should be supplied"
#endif

/*
 * uart initialization
 *
 * BAUD: 9600, 115200
 * */
int init_uart(uint32_t baud)
{

  /*
   * USART Control and Status Register A
   *
   * @features
   *  - UCSRA[1] = U2X, USART Double Transmission Speed : 1
   * */
  REG_UCSRA = _BV(U2X0); // (0b00000010)

  /*
   * USART Control and Status Register B
   *
   * @features
   *  - UCSRB[7] = RXCIE, Receive Complete Interrupt Enable : 1
   *  - UCSRB[6] = TXCIE, Transmit Complete Interrupt Enable : 1
   *  - UCSRB[5] = UDREIE, USART Data Register Empty Interrupt Enable : 0
   *  - UCSRB[4] = RXEN, Receiver Enable : 1
   *  - UCSRB[3] = TXEN, Transmitter Enable : 1
   *  - UCSRB[2] = UCSZ[2], USART Character Size : 0
   * */
  REG_UCSRB =
      _BV(RXCIE0) | _BV(TXCIE0) | _BV(RXEN0) | _BV(TXEN0); // (0b11011000)

  /*
   * USART Control and Status Register C
   *
   * @features
   *  - UCSRC[7:6] = UMSEL[1:0], USART Mode SELect : 00 = Async USART
   *  - UCSRC[3] = USBS, USART Stop Bit Select : 0 = 1bit
   *  - UCSRC[2:1] = UCSZ[1:0], USART Character Size : 11 -> 011 = 8bit
   * */
  REG_UCSRC = _BV(UCSZ01) | _BV(UCSZ00); // (0b00000110: 8bit characters)

  /*
   * USART Baud Rate Register
   *
   * @features
   *  - UBRRH: High Byte
   *  - UBRRL: Low Byte
   *
   * @formula
   *   - Async U2X Mode Baud Value: (F_CPU / 8 / baud) - 1
   * */
  uint16_t ubrr = (F_CPU / 8 / baud) - 1;
  REG_UBRRH = (ubrr >> 8);
  REG_UBRRL = ubrr;
  /* REG_UBRRH = UBRRH_VALUE; */
  /* REG_UBRRL = UBRRL_VALUE; */

  return 0;
}

// RXCI0
ISR(USART_RX_VECT)
{
  // v10. read and save at buffer.
  char status, data;

  // wait status ready.
  while (((status = REG_UCSRA) & _BV(BIT_RXC)) == 0)
    ;

  data = REG_UDR;

  // save at the buffer. check a overflow
  // indices of the _rx_buff: [0, 127]
  _rx_buff[_rx_tail] = data;
  _rx_tail = (_rx_tail + 1) % USART_BUFF_SIZE;
  _rx_len++;
}

ISR(USART_UDRE_VECT)
{
  uint8_t c;

  if (_tx_len > 0) {
    c = _tx_buff[_tx_head];
    REG_UDR = c;

    // _tx_head: [0, USART_BUFF_SIZE)
    _tx_head = (_tx_head + 1) % USART_BUFF_SIZE;
    --_tx_len;
    /* ENABLE_TXCI(); */
  }

  DISABLE_UDREI();
}

ISR(USART_TX_VECT)
{
  uint8_t c;

  if (_tx_len > 0) {
    c = _tx_buff[_tx_head];
    REG_UDR = c;

    _tx_head = (_tx_head + 1) % USART_BUFF_SIZE;
    --_tx_len;
  }
}

/*
 * @return: 1 for success, 0 for fail
 * */
int getchar_uart(uint8_t *c)
{
  DISABLE_RXCI();

  if (_rx_len > 0) {
    *c = _rx_buff[_rx_head];
    _rx_head = (_rx_head + 1) % USART_BUFF_SIZE;
    _rx_len--;

    ENABLE_RXCI();

    // TODO: How to determine the return value?
    return 1;
  }

  ENABLE_RXCI();
  return 0;
}

int putchar_uart(uint8_t c)
{
  DISABLE_TXCI();  // turn off TX complete interrupt
  DISABLE_UDREI(); // turn off data register empty interrupt
  if (_tx_len < USART_BUFF_SIZE) {
    _tx_buff[_tx_tail] = c;
    _tx_tail = (_tx_tail + 1) % USART_BUFF_SIZE;
    _tx_len++;

    // turn on data register empty interrupt *manually*
    ENABLE_UDREI();

    return 1;
  }

  ENABLE_UDREI(); // turn on data register empty interrupt
  return 0;
}
