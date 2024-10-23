#ifndef __MAIN__H
#define __MAIN__H

//==================================================
// includes
//==================================================
#include "Console.h"
#include "MA.h"
#include "putil.h"
#include "sensor.h"
#include "timer.h"
#include "uart.h"
#include "uart1.h"
#include <avr/eeprom.h>
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
/* #include <xc.h> */

//==================================================
// defines
//==================================================
#define BAUD_RATE 115200L

// on-board debugging led
#define _TOGGLE_LED() _XOR(PORTB, 7);
#define PUTCHAR_DCP(c) putchar_uart1(c)

// structs
typedef struct _cfg {
  int sv_mode;     // 0, 1
  int st_mode;     // 0, 1
  int st_interval; // 0..60
  int mon_height;  // 500 in millimeters
  char mark[4];    // "RR01"
} Config;

// declarations
void init_gpio();
void init_opto();
void init_relay();
int init_dcps();

void toggle_gpios();
void toggle_optos();
void toggle_relays();

static int load_config(Config *pCfg);
static int cfg_is_valid(const Config *pCfg);
static void init_config(Config *pCfg);
static int save_config(const Config *pCfg);
static void send_to_mc(const char *data);

// dcps
int get_dcps_packet(char *outBuff);
static void process_mc_cmd(char *cmdBuff, int cmdLen);
#endif
