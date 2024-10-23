#include <stdint.h>

static int xx_updated = 0;
static uint16_t xx_value = 6000; // 250 ~ 6000mm

int is_xx_sensor_updated() { return xx_updated; }

int get_xx_sensor_value()
{
  xx_updated = 0;
  return xx_value;
}
