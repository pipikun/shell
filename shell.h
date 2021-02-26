#ifndef __SHELL_H__
#define __SHELL_H__
#include "main.h"
#include "string.h"
#include "usart.h"

void shell_read_dma_buf(void);
void shell_init(void);

void hw_print_char(uint8_t ch);
void hw_print_str(char *str);
int shell_htoi(char *s);
char shell_tolower(char c);
void shell_data_spilt(uint8_t *src, uint16_t *tag, uint16_t *len);

#endif /*__SHELL_H__ */
//vim: ts=8 sw=8 noet autoindent:
