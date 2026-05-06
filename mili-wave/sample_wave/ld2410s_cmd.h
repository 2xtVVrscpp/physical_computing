#include <stdio.h>
#include "pico/stdlib.h"
#include "debug.h"

#ifndef _H_LD2410S_CMD_
#define _H_LD2410S_CMD_

extern void enable_config_mode(uart_inst_t *uart);
extern void end_config_mode(uart_inst_t *uart);
extern void set_generic_parameters(uart_inst_t *uart);
extern void get_version(uart_inst_t *uart);
extern void get_serial(uart_inst_t *uart);
extern int get_distance(uart_inst_t *uart);

#endif /* _H_LD2410S_CMD_ */