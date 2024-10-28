#include "SM1.h"
#include <avr/io.h>

int init_ma()
{
  PORTB |= 0x10; // ORG limit sensor : when motor runs CCW
  return 0;
}

void run_ma()
{
  run_sm1();
  /* int preStep = mac.init_step; */
}
