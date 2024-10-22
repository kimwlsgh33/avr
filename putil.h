#ifndef __PORT_UTIL_H__
#define __PORT_UTIL_H__
/*
 * PORT operation utility
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
#include <stdint.h>

#define _SET(p, b) (p |= (1 << b))
#define _CLR(p, b) (p &= ~(1 << b))
#define _XOR(p, b) (p ^= (1 << b))

#define PORT_DIR(p, value) (p = value)
#endif
