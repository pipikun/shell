#include "shell.h"

uint8_t shell_buff[SHELL_BUF_SIZE];
uint16_t shell_write_idx = 0;
uint16_t shell_select_idx = 0;

uint8_t uart_buf_byte;

void shell_split(uint8_t *src, const char *separator, uint8_t **dest, int *num)
{
	uint8_t *pNext;
	int count = 0;
	if (src == NULL || strlen((char *)src) == 0) return;
	if (separator == NULL || strlen(separator) == 0) return;
	pNext = (uint8_t *)strtok((char*)src, separator);
	while (pNext != NULL) {
		*dest++ = pNext;
		++count;
		pNext = (uint8_t *)strtok(NULL,separator);
	}
	*num = count;
}

void shell_anaysis(int argc, uint8_t *argv[])
{
	cmd_anaysis(argv, argc);
}

void shell_read_buff()
{
	uint8_t input_buff[SHELL_BUF_SIZE];
	uint8_t *src, *tag;
	uint8_t *argv[SHELL_CMD_SIZE];
	int argc;
	const char separator = ' ';

	src = shell_buff;
	tag = input_buff;
	while(*src != '\r') {
		*tag = *src;
		tag++;
		src++;
	}
	*tag = '\0';
	shell_split(input_buff, &separator, argv, &argc);
	shell_anaysis(argc, argv);
}

void shell_hardware_put(const char ch[])
{
    HAL_UART_Transmit(&UART_DEBUG_HIM, (uint8_t *)ch, strlen(ch), 100);
}

void shell_put_userinfo()
{
	shell_hardware_put(USER_LOCAL);
	shell_hardware_put(USER_SPLT);
	shell_hardware_put(USER_NAME);
	shell_hardware_put(USER_END);
}

void shell_output_buff()
{
	debug_i("\r\n");
	debug_i(" HEX -> ");
	for (int i=0; i < shell_write_idx; i++) {
		debug_i("0x%02x ", shell_buff[i]);
	} 
	debug_i("\r\n ASC -> ");

	for (int i=0; i < shell_write_idx; i++) {
		debug_i("%c", shell_buff[i]);
	} 
	debug_i("\r\n");
}

void shell_put_char(char ch)
{
	shell_buff[shell_write_idx++] = (uint8_t) ch;	
	if (ch != '\r') return;
	if (shell_write_idx >=2 && shell_buff[0] != 0x1b) {
		shell_output_buff();
		shell_read_buff();
	}
	debug_i("\r\n");
	shell_write_idx = 0;
	shell_select_idx = 0;
	shell_put_userinfo();
}

void shell_select_add(char ch) 
{
	int idx;

	idx = shell_write_idx;
	for (int i=idx; i>shell_select_idx-1; i--) {
		shell_buff[i] = shell_buff[i-1];
	}
	shell_buff[shell_select_idx-1] = ch;
	
	/* delect to the end of line */
	shell_hardware_put("\x1b[0K");
	shell_hardware_put("\33[s");
	for (int i=shell_select_idx-1; i<shell_write_idx; i++) {
		HAL_UART_Transmit(&UART_DEBUG_HIM, &shell_buff[i], 1, 0x00f);
	}
	shell_hardware_put("\33[u");
	shell_hardware_put("\33[1C"); 
}

void shell_select_del()
{
	int idx;

	idx = shell_select_idx - 1;
	for (int i=idx; i<shell_write_idx+1; i++) {
		shell_buff[i] = shell_buff[i+1];
	}
	
	/* delete to the end of line */
	shell_hardware_put("\33[1D");
	shell_hardware_put("\x1b[0K");
	shell_hardware_put("\33[s");
	for (int i=shell_select_idx - 1; i<shell_write_idx-1; i++) {
		HAL_UART_Transmit(&UART_DEBUG_HIM, &shell_buff[i], 1, 0x00f);
	}
	shell_hardware_put("\33[u");
}

void shell_ret_char(uint8_t ch)
{
	static int state = 0;

	if (state == 2) { 
		shell_write_idx -=3;
		switch (ch) {
			case KEY_UP:
				shell_hardware_put("key_up!");
				break;
			case KEY_DOWN:
				shell_hardware_put("key_down!");
				break;
			case KEY_LEFT:
				if (shell_select_idx) {
					shell_hardware_put("\33[1D");
					shell_select_idx--;
				}
				break;
			case KEY_RIGHT:
				if (shell_select_idx < shell_write_idx) {
					shell_hardware_put("\33[1C");
					shell_select_idx++;
				}
				break;
			default:
				break;

		}
		state = -1;
	}
	if (ch == '[' && state == 1) state = 2;
	if (ch == 27  && state == 0) state = 1;
	if (ch == '\b') {
		if (shell_write_idx > 1) {
			if (shell_select_idx == (shell_write_idx-1)) {
				shell_hardware_put("\b");
				shell_hardware_put("\x1b[0K");
				shell_write_idx -= 2;
				shell_select_idx-= 1;
			} else {
				if (shell_select_idx > 0) {  
					shell_select_del();
					shell_write_idx -= 2;
					shell_select_idx-= 1;
				}
			}
		} else {
			shell_write_idx = 0;
			shell_select_idx = 0;
		}
	}

	if (!state && ch > 31 && ch < 127) {
		shell_select_idx++;
		if (shell_select_idx < shell_write_idx) {
			shell_select_add(ch);	
		} else {
			HAL_UART_Transmit(&UART_DEBUG_HIM, &ch, 1, 100);
		}
	}
	if (state == -1) state = 0;
}

int tolower(int c)
{
        if (c >= 'A' && c <= 'Z') return c + 'a' - 'A';
        return c;
}

int shell_htoi(uint8_t s[])
{
        int i;
        int n = 0;
        
	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
                i = 2;
        else
                i = 0;
        for (;  (s[i] >= '0' && s[i] <= '9') || 
		(s[i] >= 'a' && s[i] <= 'z') || 
		(s[i] >= 'A' && s[i] <= 'Z'); ++i){
                if (tolower(s[i]) > '9')
                        n = 16 * n + (10 + tolower(s[i]) - 'a');
                else
                        n = 16 * n + (tolower(s[i]) - '0');
        }
        return n;
}

int __io_putchar(int ch)
{
	HAL_UART_Transmit(&UART_DEBUG_HIM, (uint8_t *)&ch, 1, 0x00f);
	return ch; 
}

int _write(int file, char *ptr, int len)
{
	int i;
	for(i = 0;i < len; i++){
		__io_putchar(*ptr++);
	}
	return len;
}

void shell_float_debug_i(float val)
{
	int n1,n2;
        n1 = (int)val;
        n2 = (int)((val-n1)*10)%10;
        debug_i("%3d.%d",n1,n2);
}


