/*
 * main.c
 *
 * Created: 10/2/2024 4:20:55 PM
 *  Author: FHT
 */ 
#define	F_CPU 16000000L
#include <util/delay.h>
#include <xc.h>

#define BAUD_RATE 115200L

#define TEST_GPIO 0
#define TEST_OPTO 0
#define TEST_UART 1
#define TEST_RELAY 0

void init_gpio();
void init_opto();
void init_relay();

void toggle_gpios();
void toggle_optos();
void toggle_relays();

void init_uart(unsigned int baud, unsigned char port);
void uart_transmit(unsigned char data, unsigned char port);
void uart_trans_string(const char *str, unsigned char port);
unsigned char uart_receive(unsigned char port);

char buf[64];
long received;

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

	// RS232 UART
	// 103: 16MHz, 9600bps, U2Xn = 0
	// 16: 16MHz, 115200bps, U2Xn = 0
#if TEST_UART
	init_uart(F_CPU/8/BAUD_RATE - 1, 0);
	init_uart(F_CPU/8/BAUD_RATE - 1, 1);
#endif

	// Relay
#if TEST_RELAY
	init_relay();
#endif
	
	//==================================================
	// function
	//==================================================
    while(1)
    {
#if TEST_GPIO
		toggle_gpios();
#endif
#if TEST_OPTO
		toggle_optos();
#endif
#if TEST_UART
		// uart_trans_string("Hello, World!\n");
		received = uart_receive(0);
		uart_transmit(received, 0); // Just back to Serial Port
		/*
		if (received != 0xFF) {	
			uart_transmit(received, 1);
			uart_trans_string("[ RX0 >> TX1 ]\n", 0);
		}

		received = uart_receive(1);
		if (received != 0xFF) {
			uart_transmit(received, 0);
			uart_trans_string("[ RX1 >> TX0 ]\n", 0);
		}
		*/
#endif
#if TEST_RELAY
		toggle_relays();
#endif
		_delay_ms(1000);
    }
}


//==================================================
// initialize
//==================================================
void init_gpio()
{
	PORTF &= ~(1 << PINF0 | 1 << PINF1 | 1 << PINF2 | 1 << PINF3 | 1 << PINF4 | 1 << PINF5 | 1 << PINF6 | 1 << PINF7);
	DDRF |= (1 << PINF0 | 1 << PINF1 | 1 << PINF2 | 1 << PINF3 | 1 << PINF4 | 1 << PINF5 | 1 << PINF6 | 1 << PINF7);
}

void init_opto()
{
	PORTA &= ~(1 << PINA4 | 1 << PINA5 | 1 << PINA6 | 1 << PINA7);
	DDRA |= (1 << PINA4 | 1 << PINA5 | 1 << PINA6 | 1 << PINA7);
}

void init_uart(unsigned int baud, unsigned char id)
{
	switch(id)
	{
		case 0:
			// set the baud rate
			// UBRRnH: USART Baud Rate Register High Byte n
			// UBRRnL: USART Baud Rate Register Low Byte n
			UBRR0H = (unsigned char)(baud >> 8);
			UBRR0L = (unsigned char)baud;
			// UCSRnA U2Xn: USART Double Transmission Speed n (Default: 0)

			// enable receiver and transmitter
			/*
			UCSRnB: USART Control and Status Register n
			features:
			- RXENn: Receiver Enable n - enables the receiver n
			- TXENn: Transmitter Enable n - enables the transmitter n
			*/
			UCSR0B = (1 << RXEN0) | (1 << TXEN0);

			// set frame format: 8data, 2stop bit
			/*
			UCSRnC: USART Control and Status Register n
			features:
			- UMSELn: Mode Select n - sets the mode of USART (default: 0 - async, 1 - sync, 2 - reserved, 3 - master spi)
			- USBSn: Stop Bit Select n - sets the number of stop bits (0: 1 stop bit, 1: 2 stop bits)
			- UCSZn: Character Size n - sets the number of data bits (0b00: 5 bits, 0b01: 6 bits, 0b10: 7 bits, 0b11: 8 bits)
			*/
			// UCSR0C = (1 << USBS0) | (3 << UCSZ00);
			UCSR0C = (3 << UCSZ00);
			break;
		case 1:
			UBRR1H = (unsigned char)(baud >> 8);
			UBRR1L = (unsigned char)baud;
			UCSR1B = (1 << RXEN1) | (1 << TXEN1);
			// UCSR1C = (1 << USBS1) | (3 << UCSZ10);
			UCSR1C = (3 << UCSZ10);
			break;
		case 2:
			UBRR2H = (unsigned char)(baud >> 8);
			UBRR2L = (unsigned char)baud;
			UCSR2B = (1 << RXEN2) | (1 << TXEN2);
			UCSR2C = (1 << USBS2) | (3 << UCSZ20);
			break;
		case 3:
			UBRR3H = (unsigned char)(baud >> 8);
			UBRR3L = (unsigned char)baud;
			UCSR3B = (1 << RXEN3) | (1 << TXEN3);
			UCSR3C = (1 << USBS3) | (3 << UCSZ30);
			break;
	}
}

void init_relay()
{
	PORTC &= ~(1 << PINC4 | 1 << PINC5 | 1 << PINC6 | 1 << PINC7);
	DDRC |= (1 << PINC4 | 1 << PINC5 | 1 << PINC6 | 1 << PINC7);
}

//==================================================
// function
//==================================================
void toggle_gpios()
{
	PORTF ^= (1 << PINF0 | 1 << PINF1 | 1 << PINF2 | 1 << PINF3 | 1 << PINF4 | 1 << PINF5 | 1 << PINF6 | 1 << PINF7);
}

void toggle_optos()
{
	PORTA ^= (1 << PINA4 | 1 << PINA5 | 1 << PINA6 | 1 << PINA7);
}

void toggle_relays()
{
	PORTC ^= (1 << PINC4 | 1 << PINC5 | 1 << PINC6 | 1 << PINC7);
}

void uart_transmit(unsigned char data, unsigned char id)
{
	switch(id)
	{
		case 0:
			// wait for empty transmit buffer
			/* 
			  UCSRnA: USART Control and Status Register n
			  features: 
			  - UDREn: USART Data Register Empty Flag n - indicates if the TXB buffer is empty
			  - RXCn: USART Receive Complete Flag n - indicates if data has been received and is available in the RXB buffer
			*/
			while ( !( UCSR0A & (1 << UDRE0)) );
			// put data to TXB buffer, sends the data
			// UDRn: USART Data(I/O) Register n
			UDR0 = data;
			break;
		case 1:
			while ( !(UCSR1A & (1 << UDRE1)) );
			UDR1 = data;
			break;
		case 2:
			while ( !(UCSR2A & (1 << UDRE2)) );
			UDR2 = data;
			break;
		case 3:
			while ( !(UCSR3A & (1 << UDRE3)) );
			UDR3 = data;
			break;
	}
}

void uart_trans_string(const char *str, unsigned char id)
{
	static unsigned char i = 0;
	while(str[i] != '\n') {
		uart_transmit(str[i++], id);
	}
	_delay_ms(1000);
}

unsigned char uart_receive(unsigned char id)
{
	switch(id)
	{
		case 0:
			while ( !(UCSR0A & (1 << RXC0)) );
			return UDR0;
		case 1:
			while ( !(UCSR1A & (1 << RXC1)) );
			return UDR1;
		case 2:
			while ( !(UCSR2A & (1 << RXC2)) );
			return UDR2;
		case 3:
			while ( !(UCSR3A & (1 << RXC3)) );
			return UDR3;
	}
	
	return 0xFF;
}