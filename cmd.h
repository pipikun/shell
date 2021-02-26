#ifndef __CMD_H__
#define __CMD_H__

#include "string.h"
#include "main.h"
#include "shell.h"

typedef struct SHELL_PARAM
{
        uint8_t *name;
        uint8_t **argv;
        uint16_t size;
        uint16_t index;
        uint16_t argc;
        const char *module;
} shell_param_t;

typedef void (*shell_func)(shell_param_t *);

typedef struct CMD_LIST
{
        shell_func entry;
        const char *name;
        const char *desc;
} cmd_list_t;

typedef struct SHELL_CMD_LIST
{
        cmd_list_t *list;
        const char *name;
        uint8_t size;
} shell_cmd_list_t;

typedef struct SHELL_CONFIG
{
        uint8_t id;
        uint8_t size;
} shell_config_t;


void shell_cmd_main_entry(uint8_t **argv, uint16_t argc);
void shell_run(uint8_t **argv, uint16_t argc);
void shell_help(shell_param_t *param);
void shell_reset(shell_param_t *param);
void shell_clear(shell_param_t *param);
void shell_module_list(shell_param_t *param);
void shell_cmd_search(uint8_t *src, uint8_t *tag, uint16_t *len);

#endif /*__CMD_H__ */
//vim: ts=8 sw=8 noet autoindent:
