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
#define timer_isfired(x) ((timer_get(x) == 0))

void init_timer();
int alloc_timer();
int timer_set(int, uint32_t);
#endif
