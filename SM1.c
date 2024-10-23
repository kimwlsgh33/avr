#include "SMC.h"

#define MDELTA_SPEED 20

static SMCtrl sm1;

void run_sm1()
{
  if (sm1.state == SM_ST_RUNNING) {
    if (sm1.mi_upFlag == 0) {
      // Calculate the next speed for running mode, until reach to target speed.

      // TODO: only when (cur_speed < mc.speed)
      if (sm1.m_nextSpeed < sm1.speed) {
        int next_speed = sm1.m_nextSpeed + MDELTA_SPEED;
        if (next_speed > sm1.speed) {
          next_speed = sm1.speed;
        }
      }
    }
  }
}
