/*
 * main.c
 *
 * Created: 10/2/2024 4:20:55 PM
 *  Author: FHT
 */ 
#define	F_CPU 16000000L
#include <util/delay.h>
#include <xc.h>

int main(void)
{
	// initialize
	PORTF |= (0x01 << PINF0 || 0x01 << PINF1 || 0x01 << PINF2 || 0x01 << PINF3 || 0x01 << PINF4 || 0x01 << PINF5 || 0x01 << PINF6 || 0x01 << PINF7);
	DDRF |= (0x01 << PINF0 | 0x01 << PINF1 | 0x01 << PINF2 | 0x01 << PINF3 | 0x01 << PINF4 | 0x01 << PINF5 | 0x01 << PINF6 | 0x01 << PINF7);
	
    while(1)
    {
		PINF |= (0x01 << PINF0 | 0x01 << PINF1 | 0x01 << PINF2 | 0x01 << PINF3 | 0x01 << PINF4 | 0x01 << PINF5 | 0x01 << PINF6 | 0x01 << PINF7);
		_delay_ms(300);
		PINF &= ~(0x01 << PINF0 | 0x01 << PINF1 | 0x01 << PINF2 | 0x01 << PINF3 | 0x01 << PINF4 | 0x01 << PINF5 | 0x01 << PINF6 | 0x01 << PINF7);
		_delay_ms(300);
    }
}