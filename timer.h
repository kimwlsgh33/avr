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
#define timer_isfired(x) ((get_timer(x) == 0))

void init_timer();
int alloc_timer();
int set_timer(int, uint32_t);
int get_timer(uint32_t);
#endif
