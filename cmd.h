#ifndef __cmd_H
#define __cmd_H

#include "main.h"

typedef void (*shell_void_func)(uint8_t ** ,uint16_t);
typedef struct shell_call
{
	const char * name;	/* the name of shell call */
	const char * desc;	/* description of shell call */
	shell_void_func func;	/* the function address of shell call */
} shell_call_t;

void cmd_anaysis(uint8_t **argv, uint16_t argc);
void shell_help(uint8_t **cmd, uint16_t num);
void shell_echo(uint8_t **cmd, uint16_t num);
uint8_t shell_cmd_strcmp(char const *src, uint8_t *tag);
void shell_reset(uint8_t **cmd, uint16_t num);
void shell_clear(uint8_t **cmd, uint16_t num);
uint8_t shell_range_addr_spilt(uint8_t *src, uint8_t * buf);
uint8_t shell_range_data_spilt(uint8_t *src, uint16_t *buf);

#endif
