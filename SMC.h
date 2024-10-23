/*
 * Stepper Motor Controller
 * -
 * */
#ifndef __SMC_H__
#define __SMC_H__
#include <stdint.h>

#define SM_ST_READY 'R'
#define SM_ST_MOVING 'm'
#define SM_ST_RUNNING 'r'
#define SM_ST_STOPPING 's'

typedef struct _sm_ctrl {
  int state;
  int speed;
  int dir;

  uint32_t cp_pos; // current position in pulse count

  int astate; // accelleration
  uint32_t min_speed;
  uint32_t max_speed;
  uint32_t m_pb;   // relative position to de-accel
  uint32_t m_dist; // distance to move to

  // next plan
  uint32_t m_prevPos;
  uint32_t m_nextSpeed;

  // updated the pos
  uint32_t m_nextPos;
  uint32_t m_nextOCR;

  // within the interrupt routine
  int mi_upFlag;
  int mi_stopFlag;
  uint32_t mi_dpos;     // relative position from start
  uint32_t mi_nextPost; // next position
} SMCtrl;

#endif
