/*
 * main.c
 *
 * Created: 10/2/2024 4:20:55 PM
 *  Author: FHT
 */
#include "main.h"
#include <string.h>

#define TEST_GPIO 1
#define TEST_OPTO 0
#define TEST_RELAY 0
#define TEST_TIMER 0
#define TEST_CONSOLE 1
#define TEST_CONFIG 0
#define TEST_UART 0
#define TEST_MA 0

static Config config;
static int tid_op;
static int tid_st; // automatic state send interval, when st_send_mode == 1

static int sv_tx_mode = 0;
static int st_tx_mode = 0;
static char tmp_rx_buff[128];

/*
 * DCPS: Data Copies Per Second
 * */
#define DCP_MAX_PACKET_LEN 128
static uint8_t rx_buff[DCP_MAX_PACKET_LEN];
static uint8_t rx_len;
#define DCP_GETCHAR(pc) getchar_uart1(pc)
static int dcpLen;
static char dcpCmd[128];

int main(void)
{
  //==================================================
  // initialize
  //==================================================
  // GPIO
#if TEST_GPIO
  init_gpio();
#endif

  // Optocoupler
#if TEST_OPTO
  init_opto();
#endif

  // Relay
#if TEST_RELAY
  init_relay();
#endif

#if TEST_CONSOLE
  init_cons(BAUD_115200);
#endif

#if TEST_TIMER
  init_timer();
  asm("sei");
  tid_op = alloc_timer();
  set_timer(tid_op, 0);
  tid_st = alloc_timer();
#endif

#if TEST_CONFIG
  load_config(&config);
  if (!cfg_is_valid(&config)) {
    init_config(&config);
    save_config(&config);
    printf("Config default !!\n");
  }
#endif

  // RS232 UART
  // 16: 16MHz, 115200bps, U2Xn = 1
#if TEST_UART
  init_dcps();
#endif

  //==================================================
  // loop
  //==================================================
  while (1) {
#if TEST_GPIO
    toggle_gpios();
#endif
#if TEST_OPTO
    toggle_optos();
#endif
#if TEST_RELAY
    toggle_relays();
#endif
#if TEST_CONSOLE
    printf("CONSOLE WORKS v1.0 \n");
#endif
#if TEST_TIMER
    if (timer_isfired(tid_op)) {
      set_timer(tid_op, 500);
      _TOGGLE_LED();

      if (sv_tx_mode == 1) {
        if (is_xx_sensor_updated()) {
          // automatic clear is updated.
          int sv = get_xx_sensor_value();

          sprintf(tmp_rx_buff, "SS=%d", sv);
          send_to_mc(tmp_rx_buff);
        }
      }
    }
#endif
#if TEST_MA
    run_ma();
#endif
#if TEST_UART
    if ((dcpLen = get_dcps_packet(dcpCmd)) > 0) {
      process_mc_cmd(dcpCmd, dcpLen);
    }

    if ((dcpLen = get_cons_cmdline(dcpCmd)) > 0) {
    }
#endif
    _delay_ms(1000);
  }
}

//==================================================
// initialize
//==================================================
void init_gpio()
{
  PORTF &= ~(1 << PINF0 | 1 << PINF1 | 1 << PINF2 | 1 << PINF3 | 1 << PINF4 |
             1 << PINF5 | 1 << PINF6 | 1 << PINF7);
  DDRF |= (1 << PINF0 | 1 << PINF1 | 1 << PINF2 | 1 << PINF3 | 1 << PINF4 |
           1 << PINF5 | 1 << PINF6 | 1 << PINF7);
}

void init_opto()
{
  PORTA &= ~(1 << PINA4 | 1 << PINA5 | 1 << PINA6 | 1 << PINA7);
  DDRA |= (1 << PINA4 | 1 << PINA5 | 1 << PINA6 | 1 << PINA7);
}

void init_relay()
{
  PORTC &= ~(1 << PINC4 | 1 << PINC5 | 1 << PINC6 | 1 << PINC7);
  DDRC |= (1 << PINC4 | 1 << PINC5 | 1 << PINC6 | 1 << PINC7);
}

// Data Copies Per Second
int init_dcps()
{
  init_uart1(BAUD_115200);
  return 0;
}

//==================================================
// function
//==================================================
void toggle_gpios()
{
  PORTF ^= (1 << PINF0 | 1 << PINF1 | 1 << PINF2 | 1 << PINF3 | 1 << PINF4 |
            1 << PINF5 | 1 << PINF6 | 1 << PINF7);
}

void toggle_optos()
{
  PORTA ^= (1 << PINA4 | 1 << PINA5 | 1 << PINA6 | 1 << PINA7);
}

void toggle_relays()
{
  PORTC ^= (1 << PINC4 | 1 << PINC5 | 1 << PINC6 | 1 << PINC7);
}

int send_dcps(const char *packet, int len)
{
  int i;

  for (i = 0; i < len; ++i) {
    PUTCHAR_DCP(packet[i]);
  }

  PUTCHAR_DCP('\r'); // CR
  PUTCHAR_DCP('\n'); // LF

  return 0;
}

/*
 * @return: length of packet
 * */
int get_dcps_packet(char *outBuff)
{
  uint8_t c;
  /* (DCP_GETCHAR((unsigned char *)&c) == 1) */
  // HACK: Why not use the same type?
  if (DCP_GETCHAR(&c)) {
    // packet end
    if (c == '\r') {
      int len = rx_len;

      rx_buff[rx_len] = 0;
      memcpy(outBuff, rx_buff, rx_len + 1);
      rx_len = 0;

      return len;
    } else {
      // payloads
      if (rx_len < (DCP_MAX_PACKET_LEN - 1)) {
        rx_buff[rx_len++] = c;
      }

      return -1;
    }
  }

  return -1;
}

//==================================================
// static functions
//==================================================

// load config from eeprom
static int load_config(Config *pCfg)
{
  asm("cli"); // CLear global Interrupt
  eeprom_read_block(pCfg, (const void *)0, sizeof(Config));
  asm("sei"); // Set global External Interrupt

  return 0;
}

static int is_cfg_valid(const Config *pCfg)
{
  if ((pCfg->sv_mode < 0) || (pCfg->sv_mode > 1))
    return 0;
  if ((pCfg->st_mode < 0) || (pCfg->st_mode > 1))
    return 0;
  if ((pCfg->st_interval < 1) || (pCfg->st_interval > 60))
    return 0;
  if ((pCfg->mon_height < 1) || (pCfg->mon_height > 700))
    return 0;

  if (memcmp(pCfg->mark, "RR01", 4) != 0)
    return 0;

  return 1;
}

// initiate the config
static void init_config(Config *pCfg)
{
  printf("init default settings\n");
  pCfg->sv_mode = 0;
  pCfg->st_mode = 0;
  pCfg->st_interval = 0;
  pCfg->mon_height = 500;

  memcpy(pCfg->mark, "RR01", 4);
}

// store the config
static int save_config(const Config *pCfg)
{
  asm("cli");
  eeprom_write_block(pCfg, (void *)0, sizeof(Config));
  asm("sei");

  return 1;
}

static void send_to_mc(const char *data)
{
  int len = strlen(data);
  send_dcps(data, len);
}

/*
 * @feature Parse the User Command and send it to the MC
 * */
static void process_mc_cmd(char *cmdBuff, int cmdLen)
{

  if ((cmdLen <= 0) || (cmdBuff == 0))
    return;

  printf("mc cmd: %s\n", cmdBuff);

  // NOTE: Create the commands!
  if (strcmp(cmdBuff, "ID?") == 0) {
    send_to_mc("RC v1.0");
  }
}
