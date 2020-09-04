#include "shell.h"

uint8_t shell_buff[SHELL_BUF_SIZE];
uint16_t shell_write_idx = 0;
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

void shell_put_userinfo()
{
	uint8_t info_local[] = USER_LOCAL;
	uint8_t info_splt[] = USER_SPLT;
	uint8_t info_name[] = USER_NAME;
	uint8_t info_end[] = USER_END;

	HAL_UART_Transmit(&UART_DEBUG_HIM, (uint8_t *)info_local, strlen((const char *)info_local), 100);
	HAL_UART_Transmit(&UART_DEBUG_HIM, (uint8_t *)info_splt, strlen((const char *)info_splt), 100);
	HAL_UART_Transmit(&UART_DEBUG_HIM, (uint8_t *)info_name, strlen((const char *)info_name), 100);
	HAL_UART_Transmit(&UART_DEBUG_HIM, (uint8_t *)info_end, strlen((const char *)info_end), 100);
}

void shell_put_char(char ch)
{
	shell_buff[shell_write_idx++] = (uint8_t) ch;
	if (ch != '\r') return;

	if (shell_write_idx >=2)
		shell_read_buff();
	else
		debug_i("\r\n");
	shell_write_idx = 0;
	shell_put_userinfo();
}

void shell_hardware_put(char ch[])
{
    HAL_UART_Transmit(&UART_DEBUG_HIM, (uint8_t *)ch, strlen(ch), 100);
}

void shell_ret_char(uint8_t ch)
{
	static int state = 0;

	if (state == 2) state = -1;
	if (ch == '[' && state == 1) state = 2;
	if (ch == 27  && state == 0)  state = 1;

	if(ch == '\b'){
		if(shell_write_idx > 0) {
			shell_hardware_put("\b");
			shell_hardware_put("\x1b[0K");
			shell_write_idx -= 2;
		}
	}

	if (!state && ch > 31 && ch < 127)
		HAL_UART_Transmit(&UART_DEBUG_HIM, &ch, 1, 100);
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


