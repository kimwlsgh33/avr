/*
 * Timer utility
 *
 * v0.1
 *  - timer interrupt initialization
 *  - timer application utility.
 *
 * @project gcodeLib
 * @author gcode@FHT
 * @date 2024/10/18
 */
#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>
#define timer_is_fired(x) ((get_timer(x) == 0))

/*
 * @feature: set timer mode(when the interrupt occurs) and clock
 * */
void init_timer();
int alloc_timer();

// @return: 0 for success, -1 for fail
int set_timer(int id, uint32_t value);
int get_timer(int id);
#endif
