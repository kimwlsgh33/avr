/*
 * buffer initialization for default uart
 * */
// HACK: Why we should set to 128?
#include "uart.h"
#include "putil.h"
#include <avr/io.h>
#include <stdint.h>

#define UART_BUFF_SIZE 128
static uint8_t _rx_buff[UART_BUFF_SIZE];
static uint8_t _rx_head = 0;
static uint8_t _rx_tail = 0;
static uint8_t _rx_len = 0;

static uint8_t _tx_buff[UART_BUFF_SIZE];
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
#define UART_RX_VECT USART0_RX_vect
#define UART_TX_VECT USART0_TX_vect
#define UART_UDRE_VECT USART0_UDRE_vect

// for uart0
#define DISABLE_RXCI() _CLR(UCSR0B, RXCIE0)
#define ENABLE_RXCI() _SET(UCSR0B, RXCIE0)

#define DISABLE_TXCI() _CLR(UCSR0B, TXCIE0)
#define ENABLE_TXCI() _SET(UCSR0B, TXCIE0)

#define DISABLE_UDREI() _CLR(UCSR0B, UDRIE0)
#define ENABLE_UDREI() _SET(UCSR0B, UDRIE0)

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
  REG_UCSRA = 0x02; // (0b00000010)

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
  REG_UCSRB = 0xD8; // (0b11011000)

  /*
   * USART Control and Status Register C
   *
   * @features
   *  - UCSRC[7:6] = UMSEL[1:0], USART Mode SELect : 00 = Async USART
   *  - UCSRC[3] = USBS, USART Stop Bit Select : 0 = 1bit
   *  - UCSRC[2:1] = UCSZ[1:0], USART Character Size : 11 -> 011 = 8bit
   * */
  REG_UCSRC = 0x6; // (0b00000110: 8bit characters)

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
  REG_UBRRH = ((F_CPU / 8 / baud) - 1) >> 8;
  REG_UBRRL = ((F_CPU / 8 / baud) - 1);
  return 0;
}

// RXCI0
ISR(UART_RX_VECT)
{
  // v10. read and save at buffer.
  char status, data;

  // wait status ready.
  while (((status = REG_UCSRA) & (1 << BIT_RXC)) == 0)
    ;

  data = REG_UDR;

  // save at the buffer. check a overflow
  // indices of the _rx_buff: [0, 127]
  _rx_buff[_rx_tail] = data;
  _rx_tail = (_rx_tail + 1) % UART_BUFF_SIZE;
  _rx_len++;
}

/*
 * @return: 1 for success, 0 for fail
 * */
int getchar_uart(uint8_t *c)
{
  DISABLE_RXCI();

  if (_rx_len > 0) {
    *c = _rx_buff[_rx_head];
    _rx_head = (_rx_head + 1) % UART_BUFF_SIZE;
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
  if (_tx_len < UART_BUFF_SIZE) {
    _tx_buff[_tx_tail] = c;
    _tx_tail = (_tx_tail + 1) % UART_BUFF_SIZE;
    _tx_len++;

    ENABLE_UDREI();

    return 1;
  }

  ENABLE_UDREI(); // turn on data register empty interrupt
  return 0;
}
