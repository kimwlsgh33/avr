/*
 * Moving Actuator with Stepper Moter
 * - find initial position
 * - run CW, CWW with fixed speed only
 * */
#ifndef __MOV_ACT_H__
#define __MOV_ACT_H__

#include <stdint.h>

/*
 * individual motor control state just cw, ccw
 * Simplest DC motor controller
 * */
typedef struct _ma_ctrl {
  int state;
  int speed;
  int dir;

  uint32_t pos;

  int ini_step;
  uint32_t trgt_pos;
} MACtrl;

void run_ma();
#endif
