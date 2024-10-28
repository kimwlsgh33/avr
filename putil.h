#ifndef __PORT_UTIL_H__
#define __PORT_UTIL_H__
/*
 * PORT(sfr) operation utility
 * - SFR : Special Function Register
 *
 * v0.1
 *  - bit operations are added
 *  - port macro is added
 *
 * @project gcodeLib
 * @author gcode@FHT
 * @date 2024/10/15
 * */

// CMD(Port, Bit)
#include <avr/sfr_defs.h>
#include <stdint.h>

/*
 * Init State: #define sbi(sfr, bit) (sfr |= (1 << bit))
 *
 * BV: Bit Value
 * */
#define cbi(sfr, bit) (sfr &= ~_BV(bit))
#define sbi(sfr, bit) (sfr |= _BV(bit))
#define tbi(sfr, bit) (sfr ^= _BV(bit))

#endif
