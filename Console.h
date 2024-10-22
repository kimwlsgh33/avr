/*
 * Console
 */

#ifndef __CONSOLE_H_
#define __CONSOLE_H_

// #include "BuildOptions.h"
#define USE_CONSOLE

#include <stdint.h>
void init_cons(uint32_t baud);
void enable_cons();
void disable_cons();

/*
 * Copyout command
 *
 * @return bufferedcommand length
 *   -1: not yet.
 */
int get_cmd_ln_cons(char *poutBuff);

#endif
