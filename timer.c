/*
 *   initialize timer
 *     - timer utility init
 *     - timer register setting
 */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#define MAX_SYS_TIMER 16

#ifdef F_CPU
#define CPUCLK F_CPU
#else
#define CPUCLK 16000000L
#endif

// 1ms interval, 1000 tick per seconds
#define TIMER_TICK 1000
#define TIMER_TCNT (CPUCLK / 256 / TIMER_TICK)

typedef struct _sys_timer {
  uint8_t flag; // 0x80: assigned,  0x01: running
  uint32_t value;
} sys_timer;

sys_timer timer_list[MAX_SYS_TIMER];
uint32_t timerCount;

void init_timer()
{
  // initialize all timer
  for (int i = 0; i < MAX_SYS_TIMER; ++i) {
    timer_list[i].flag = 0;
    timer_list[i].value = 0;
  }

  /*
   * Timer/Counter 1 High & Low (16 bits)
   * */
  TCNT1H = 0;
  TCNT1L = 0;

  /*
   * Waveform Generation Mode - WGMn[3:0]
   * ( counting sequence setting )
   *
   * 0100 : Clear Timer on Compare match (CTC) mode
   * */

  /*
   * Timer/Counter Control Register 1 A
   *
   * @features
   *  - TCCR1A[1:0] = WGM1[1:0] : 00
   *  - TCCR1A[3:2] = COM1C[1:0], Compare Output Mode for Channel C : 0 0
   *  - TCCR1A[5:4] = COM1B[1:0], Compare Output Mode for Channel B : 0 0
   *  - TCCR1A[7:6] = COM1A[1:0], Compare Output Mode for Channel A : 0 0
   * */
  TCCR1A = 0x00;

  /*
   * Timer/Counter Control Register 1 B
   *
   * 0x0C = 0b00001100 = TCCR1B[3:2]
   *
   * @features
   *  - TCCR1B[2:0] = CS1[2:0], Clock Select 1 : 100 : clk/256 (prescaler)
   *  - TCCR1B[4:3] = WGM1[3:2] : 01
   *
   * clk/256 = 16MHz / 256 = 62.5kHz = 62.5k per second
   * clock resolution = 1/62.5k = 0.000016s = 0.016ms = 16us
   *
   * */
  /* TCCR1B |= (1 << CS12 | 1 << WGM12); */
  TCCR1B = 0x0C;

  /*
   * Output Compare Register 1 A High/Low
   * ( set the value to compare match )
   *
   * TIMER_TCNT = CPUCLK / PRESCALER_FACTOR / TIMER_TICK
   * ( 1ms = TIMER_CLK / 1000 = 16M / 256 / 1000 )
   * */
  OCR1AH = 0;
  OCR1AL = TIMER_TCNT;

  /*
   * Timer/Counter1 Interrupt MaSK
   *
   * @features
   *  - OCIE1A: Output Compare Interrupt Enable for Channel A
   * */
#if defined(TIMSK1)
  TIMSK1 |= (1 << OCIE1A);
#else
  TIMSK |= 0x10;
#endif
}

int alloc_timer()
{
  int id;
  for (id = 0; id < MAX_SYS_TIMER; ++id) {
    if ((timer_list[id].flag & 0x80) == 0) {
      timer_list[id].flag |= 0x80;
      return id;
    }
  }

  return -1;
}

int timer_set(int id, uint32_t value)
{
  if ((id < 0) || (id >= MAX_SYS_TIMER)) {
    return -1;
  }

  if (timer_list[id].flag & 0x80) {
    timer_list[id].value = value;
    timer_list[id].flag = 0x81;
    return 0;
  }

  return -1;
}

// Timer/Counter Compare Match A
ISR(TIMER1_COMPA_vect)
{
  int i;

  // Need not clear TCNT1, it is automatically cleard on CTC mode.
  // TCNT1 = 0; 1ms timer
  ++timerCount;

  // timer decreasing.
  for (i = 0; i < MAX_SYS_TIMER; ++i) {
    if (timer_list[i].value > 0) {
      --timer_list[i].value;
    }
  }
}
