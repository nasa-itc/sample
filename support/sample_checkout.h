#ifndef _SAMPLE_CHECKOUT_H_
#define _SAMPLE_CHECKOUT_H_

/*
** Includes
*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

#include "hwlib.h"
#include "device_cfg.h"
#include "sample_device.h"

/*
** Defines
*/
#define CMD_UNKNOWN -1
#define CMD_HELP 0
#define CMD_EXIT 1
#define CMD_SEND_RW_TRQ 2
#define CMD_SEND_MTR_PWR 3
#define CMD_GET_HK 4

#define PROMPT                  "sample> "
#define MAX_INPUT_BUF         	512
#define MAX_INPUT_TOKENS      	64
#define MAX_INPUT_TOKEN_SIZE  	50
#define TELEM_BUF_LEN           8

/*
** Prototypes
*/
void delay_ms(uint32_t ms_delay);
void print_help(void);
void to_lower(char* str);
int  get_command(const char* str);
int  main(int argc, char *argv[]);

#endif /* _SAMPLE_CHECKOUT_H_ */
