#include "cmd.h"
#include "string.h"

shell_call_t func_list[] =
{
	{"list", "show the command list", shell_help},
	{"echo", "give you a echo", shell_echo},
	{"reset", "software reset the mcu",shell_reset},
	{"clear", "clear the terminal",shell_clear},
};

void shell_help(uint8_t **cmd, uint16_t num)
{
	int len;
	debug_i("Mars Shell Command List:\r\n\r\n");
	len=(sizeof(func_list)/sizeof(func_list[0]));
	for (int i=0;i<len;i++) { 
		debug_i("%s\t\t\t- %s\r\n",func_list[i].name,func_list[i].desc);
	}
}

void shell_echo(uint8_t **cmd, uint16_t num)
{
	int i = 1;
	while(i <= num-1){ 
		debug_i("\033[34m%s\033[0m ", cmd[i++]);
	}
}

void shell_reset(uint8_t **cmd, uint16_t num)
{
	shell_clear(cmd, num);
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}

void shell_clear(uint8_t **cmd, uint16_t num)
{
	debug_i("\033[2J");
	debug_i("\t\t\t\tV[0_0]V\r\n");
}

void cmd_anaysis(uint8_t **argv, uint16_t argc)
{
	int len=(sizeof(func_list)/sizeof(func_list[0]));
	debug_i("\r\n");
	for (int i=0; i<len; i++) {
		if (shell_cmd_strcmp(func_list[i].name, argv[0]) == 1) {
			func_list[i].func(argv, argc);
			debug_i("\r\n");
			return;
		}
	}
	debug_i("\r\nCommand '%s' not found \r\n\r\n", argv[0]);
}

uint8_t shell_cmd_strcmp(char const *src, uint8_t *tag)
{
	while (*src!='\0') {
		if(*src!=*tag) return 0;
		src++;
		tag++;
	}
	if (*tag != '\0') return 0;
	
	return 1;
}

uint8_t shell_range_addr_spilt(uint8_t *src, uint8_t * buf)
{
	uint8_t src_t[SHELL_BUF_SIZE];
	uint8_t *sp = src_t;
	uint8_t len = strlen((const char *)src);
	uint8_t *par[SHELL_CMD_SIZE];
	int pac;
	
	for (int i=1; i<len-1; i++) {
		*sp = src[i];
		 sp++;
	}
	shell_split(src_t, ":", par, &pac);
	buf[0] = shell_htoi(par[0]);
	buf[1] = shell_htoi(par[1]);

	return (uint8_t) pac;
}	

uint8_t shell_range_data_spilt(uint8_t *src, uint16_t *buf)
{
	uint8_t src_t[SHELL_BUF_SIZE];
	uint8_t *sp = src_t;
	uint8_t len = strlen((const char *)src);
	uint8_t *par[SHELL_CMD_SIZE];
	int pac;

	for (int i=1; i<len; i++) {
		*sp = src[i];
		 sp++;
	}
	shell_split(src_t, ",", par, &pac);
	for(int i=0; i< pac; i++) {
		buf[i] = shell_htoi(par[i]);
		debug_i("data[%d] 0x%04x\r\n",i,buf[i]);
	}
	
	debug_i("size:%d\r\n", pac);
	return pac;
}

