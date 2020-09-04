#ifndef __shell_H
#define __shell_H

#include "main.h"
#include "string.h"
#include "usart.h"

extern uint8_t uart_buf_byte;

void shell_split(uint8_t *src, const char *separator, uint8_t **dest, int *num);
void shell_anaysis(int argc, uint8_t *argv[]);
void shell_read_buff();
void shell_put_char(char ch);
void shell_ret_char(uint8_t ch);
void shell_float_debug_i(float val);
void shell_select_add(char ch);
void shell_select_del();
int shell_htoi(uint8_t s[]);


#endif
