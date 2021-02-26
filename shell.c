#include "shell.h"
#include "config.h"
#include "stdbool.h"
#include "cmd.h"
#include "mdio.h"
#include "user.c"
#include "i2c_software.h"

uint8_t uart_buf[SHELL_BUF_SIZE];
uint8_t shell_buf[SHELL_BUF_SIZE];
uint8_t history_buf[SHELL_HISTORY_SIZE][SHELL_BUF_SIZE];
uint8_t uart_buf_byte;

uint32_t shell_write_idx = 0;
uint32_t shell_select_idx = 0;
uint32_t his_write_idx = 0;
uint32_t his_read_idx = 0;
uint32_t dma_read_idx = 0;

bool key_up_down_logic = false;

inline void shell_init(void)
{
        HAL_UART_Receive_DMA(&UART_DEBUG_HIM, uart_buf, SHELL_BUF_SIZE);
        /* init history*/
        for (int i=0; i<SHELL_HISTORY_SIZE; i++) {
                for (int j=0; j<SHELL_BUF_SIZE; j++) {
                        history_buf[i][j] = 0;
                }
        }
        hw_print_str("\x1b[2K");
        hw_print_str("\33[100D");
        hw_print_str(USER_INFO);

        //module init
        i2c_module_init();
        mdio_module_init();
}

static void shell_split(uint8_t *src, const char *separator, uint8_t **dest, uint16_t *num)
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

void shell_output_cmd_buf(void)
{
        debug_i("- HEX -> ");
        for (int i=0; i < shell_write_idx; i++) {
                debug_i("0x%02x ", shell_buf[i]);
        }
        debug_i("\r\n");
        debug_i("- ASC -> ");
        for (int i=0; i < shell_write_idx; i++) {
                debug_i("%c", shell_buf[i]);
        }
        debug_i("\r\n");
        debug_i("his write idx:%ld\r\n", his_write_idx);
        debug_i("his read idx:%ld\r\n", his_read_idx);
}

static void shell_move_history(void)
{
        for (int i=0; i<SHELL_HISTORY_SIZE-2; i++) {
                for (int j=0; j< SHELL_BUF_SIZE; j++) {
                        history_buf[i][j] = history_buf[i+1][j];
                }
        }
}

static void shell_history_add(void)
{
        uint8_t *src, *tag;

        if (key_up_down_logic == true) {
                his_write_idx = his_read_idx;
                return;
        }

        if (his_read_idx == SHELL_HISTORY_SIZE - 2) {
                shell_move_history();
                his_write_idx = SHELL_HISTORY_SIZE - 3;
        }

        src = shell_buf;
        tag = &history_buf[his_write_idx++][0];
        if (his_write_idx >= SHELL_HISTORY_SIZE - 1) {
                his_write_idx = 0;
        }

        while (*src != '\n') {
                *tag = *src;
                if (*tag == 27) break; // cmd hard
                tag++;
                src++;
        }
        if (his_write_idx > his_read_idx) his_read_idx = his_write_idx;
        *tag = '\0';
}

static void shell_buf_read(void)
{
        uint8_t inp_buf[SHELL_BUF_SIZE];
        uint8_t *src, *tag;
        uint8_t *argv[SHELL_CMD_SIZE];
        uint16_t argc;

        src = shell_buf;
        tag = inp_buf;
        while (*src != '\r') {
                *tag = *src;
                tag++;
                src++;
        }
        *tag = '\0';
        shell_split(inp_buf, SHELL_SPLIT_CHAR, argv, &argc);
        shell_cmd_main_entry(argv, argc);
}

static void shell_put_char(char ch)
{
        shell_buf[shell_write_idx++]  = (uint8_t)ch;
        if (ch != '\r') return;
        hw_print_str("\r\n");
        if (shell_write_idx > 1 && shell_buf[0] != 0x1b) {
                shell_buf_read();
                shell_history_add();
        }
        shell_write_idx = 0;
        shell_select_idx = 0;
        hw_print_str(USER_INFO);
}

static void shell_select_add(char ch)
{
        int idx;

        idx = shell_write_idx;
        for (int i=idx; i>shell_select_idx-1; i--) {
                shell_buf[i] = shell_buf[i-1];
        }
        shell_buf[shell_select_idx-1] = ch;
        /* delect to the end of line */
        hw_print_str("\x1b[0K");
        hw_print_str("\33[s");
        for (int i=shell_select_idx-1; i<shell_write_idx; i++) {
                HAL_UART_Transmit(&UART_DEBUG_HIM, &shell_buf[i], 1, 0x00f);
        }
        hw_print_str("\33[u");
        hw_print_str("\33[1C");
}

void shell_select_del()
{
        int idx;

        idx = shell_select_idx - 1;
        for (int i=idx; i<shell_write_idx+1; i++) {
                shell_buf[i] = shell_buf[i+1];
        }
        /* delete to the end of line */
        hw_print_str("\33[1D");
        hw_print_str("\x1b[0K");
        hw_print_str("\33[s");
        for (int i=shell_select_idx - 1; i<shell_write_idx-1; i++) {
                HAL_UART_Transmit(&UART_DEBUG_HIM, &shell_buf[i], 1, 0x00f);
        }
        hw_print_str("\33[u");
}

static void shell_delete_key(void)
{
        if (shell_write_idx > 1) {
                if (shell_select_idx == (shell_write_idx-1)) {
                        hw_print_str("\b");
                        hw_print_str("\x1b[0K");
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

static void shell_key_update(void)
{
        /* load history */
        for (int i=0; i<SHELL_BUF_SIZE; i++) {
                shell_buf[i] = history_buf[his_write_idx][i];
        }
        /* push data */
        for (int i=0; i<shell_select_idx; i++) {
                hw_print_str("\33[1D");
        }
        hw_print_str("\33[K");
        /* output history */
        for (int i=0; i<SHELL_BUF_SIZE; i++) {
                if (shell_buf[i] == 0 || shell_buf[i] == '\r') {
                        shell_select_idx = i;
                        shell_write_idx = i;
                        break;
                }
                hw_print_char(shell_buf[i]);
        }
}

static void shell_key_down(void)
{
        if (his_write_idx >= his_read_idx-1) return;
        his_write_idx++;
        shell_key_update();
}

static void shell_key_up(void)
{
        /* save input */
        if (his_write_idx == 0) return;
        if (shell_write_idx >0 && !key_up_down_logic) {
                shell_history_add();
                his_write_idx--;
        }
        his_write_idx--;
        shell_key_update();
}

static void shell_direct_key(uint8_t ch)
{
        shell_write_idx -=3;
        switch (ch) {
                case KEY_UP:
                        shell_key_up();
                        break;
                case KEY_DOWN:
                        shell_key_down();
                        break;
                case KEY_LEFT:
                        hw_print_str("\33[1D");
                        shell_select_idx--;
                        break;
                case KEY_RIGHT:
                        if (shell_select_idx < shell_write_idx) {
                                hw_print_str("\33[1C");
                                shell_select_idx++;
                        }
                        break;
                default:
                        break;
        }
        key_up_down_logic = true;
}

static void shell_auto_cmd(void)
{
        hw_print_str("not support!");
        /* search cmd use input str */

}

static void shell_ret_char(uint8_t ch)
{
        static int state = 0;

        if (state == 2) {
                shell_direct_key(ch);
                state = -1;
        }
        if (ch == '[' && state == 1) state = 2;
        if (ch == 27  && state == 0) state = 1;
        if (ch == '\b')  shell_delete_key();
        if (ch == '\t')  shell_auto_cmd();
        if (!state && ch > 31 && ch < 127) {
                key_up_down_logic = false;
                shell_select_idx++;
                if (shell_select_idx < shell_write_idx) {
                        shell_select_add(ch);
                } else {
                        HAL_UART_Transmit(&UART_DEBUG_HIM, &ch, 1, 100);
                }
        }
        if (state == -1) state = 0;
}

void shell_read_dma_buf(void)
{
        uint32_t state, idx;

        state = __HAL_UART_GET_FLAG(&UART_DEBUG_HIM, UART_FLAG_IDLE);
        if (state != RESET) {
                __HAL_UART_CLEAR_IDLEFLAG(&UART_DEBUG_HIM);
                idx = SHELL_BUF_SIZE - UART_DMA_RX.Instance->CNDTR;
                while (dma_read_idx != idx) {
                        char c = uart_buf[dma_read_idx++];
                        if (dma_read_idx == SHELL_BUF_SIZE) dma_read_idx = 0;
                        shell_put_char(c);
                        shell_ret_char(c);
                }
        }
}

char shell_tolower(char c)
{
        if (c >= 'A' && c <= 'Z') {
                return c + 'a' - 'A';
        }
        return c;
}

int shell_htoi(char *s)
{
        int i, n = 0;

        if (s[0] == '0' && (s[1]=='x' || s[1]=='X')) {
                i = 2;
        } else {
                i = 0;
        }

        for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i) {
                if (shell_tolower(s[i]) > '9') {
                        n = 16 * n + (10 + shell_tolower(s[i]) - 'a');
                } else {
                        n = 16 * n + (shell_tolower(s[i]) - '0');
                }
        }

        return n;
}

void shell_data_spilt(uint8_t *src, uint16_t *tag, uint16_t *len)
{
        uint8_t *buf[SHELL_BUF_SIZE];

        shell_split(src,",", buf, len);
        for (uint16_t i=0; i<*len; i++) {
                *tag = shell_htoi((char *)buf[i]);
                tag++;
        }
}

void hw_print_char(uint8_t ch)
{
        HAL_UART_Transmit(&UART_DEBUG_HIM, &ch, 1, 100);
}

void hw_print_str(char *str)
{
        HAL_UART_Transmit(&UART_DEBUG_HIM, (uint8_t*)str, strlen(str), 100);
}

int __io_putchar(char ch)
{
        HAL_UART_Transmit(&UART_DEBUG_HIM, (uint8_t *)&ch, 1, 100);
        return ch;
}

int _write(int file, char *ptr, int len)
{
        for(int i = 0;i < len; i++){
                __io_putchar(*ptr++);
        }
        return len;
}

