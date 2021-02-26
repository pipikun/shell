#include "cmd.h"
#include "user.h"
#include "i2c_software.h"
#include "mdio.h"
#include "flash.h"
#include "user.h"

/* The shell mode cmd list*/
cmd_list_t shell_cmd[] =
{
        {.entry = shell_help,
         .name = "?",
         .desc = "Display this information"
        },
        {.entry = shell_clear,
         .name = "clear",
         .desc = "Clear the terminal"
        },
        {.entry = shell_reset,
         .name = "reset",
         .desc = "Reset the system"
        },
        {.entry = shell_module_list,
         .name = "list",
         .desc = "Display support module"
        },
};

/* User module register */
cmd_list_t i2c_cmd[] =
{
        {.entry = shell_help,
         .name =  "?",
         .desc = "Output help information"
        },
        {.entry = i2c_read,
         .name = "-r",
         .desc = "I2C master read data"
        },
        {.entry = i2c_write,
         .name = "-w",
         .desc = "I2C master write data"
        },
        {.entry = i2c_read_range,
         .name = "-R",
         .desc = "I2C master read data with range"
        },
        {.entry = i2c_write_range,
         .name = "-W",
         .desc = "I2C master write data with range"
        },
        {.entry = i2c_module_info,
         .name = "-i",
         .desc = "Get i2c module information"
        },
        {.entry = shell_module_list,
         .name = "list",
         .desc = "Display support module"
        },
};

cmd_list_t mdio_cmd[] =
{
        {.entry = shell_help,
         .name = "?",
         .desc = "Output help information"
        },
        {.entry = mdio_clause_setting,
         .name = "-c",
         .desc = "Clause setting"
        },
        {.entry = mdio_phy_addr_setting,
         .name = "-p",
         .desc = "Phy addr setting"
        },
        {.entry = mdio_dev_type_setting,
         .name = "-d",
         .desc = "Dev type setting"
        },
        {.entry = mdio_read,
         .name = "-r",
         .desc = "MDIO read data"
        },
        {.entry = mdio_write,
         .name = "-w",
         .desc = "MDIO write data"
        },
        {.entry = mdio_info,
         .name = "-i",
         .desc = "MDIO module information"
        }
};

cmd_list_t flash_cmd[] =
{
        {.entry = shell_help,
         .name = "?",
         .desc = "Output help information."
        },
        {.entry = flash_read,
         .name = "-r",
         .desc = "Read one page data."
        },
        {.entry = flash_write,
         .name = "-w",
         .desc = "Write one page data."
        },
        {.entry = flash_info,
         .name = "-i",
         .desc = "Output flash module information."
        }
};

/* ----- end line ----- */

/* Shell mode command list */
shell_cmd_list_t shell_list[] =
{
        {.list = shell_cmd,
         .name = "shell",
         .size = sizeof(shell_cmd) / sizeof(shell_cmd[0])
        },
        {.list = i2c_cmd,
         .name = "i2c",
         .size =  sizeof(i2c_cmd)/sizeof(i2c_cmd[0])
        },
        {.list = mdio_cmd,
         .name = "mdio",
         .size =  sizeof(mdio_cmd)/sizeof(mdio_cmd[0])
        },
        {.list = flash_cmd,
         .name = "flash",
         .size =  sizeof(flash_cmd)/sizeof(flash_cmd[0])
        },
};

/* Shell config data */
shell_config_t shell_config =
{
        .id = 0,
        .size = sizeof(shell_list)/sizeof(shell_list[0])
};

/* Shell command compare */
static uint8_t shell_strcmp(const char *src, uint8_t *tag)
{
        while (*src!='\0') {
                if(*src++ != *tag++) return 0;
        }
        if (*tag != '\0' && *tag != ' ') return 0;
        return 1;
}

/* The public function of run module  */
void shell_run_entry(cmd_list_t *list, shell_param_t *param)
{
        for (int i=0; i<param->size; i++) {
                if (shell_strcmp(list[i].name, param->name)) {
                        list[i].entry(param);
                        return;
                }
        }
        if (param->name[0] == '-') {
                debug_s("\r\n%s: error, unrecognized command line option '%s' \r\n\r\n", param->module, param->name);
        }
}

/* shell help */
void shell_help(shell_param_t *param)
{
        uint8_t id = shell_config.id;
        uint8_t size = shell_list[id].size;
        cmd_list_t *list = shell_list[id].list;

        debug_s("Usage: %s [options]\r\n", shell_list[id].name);
        debug_s("Options:\r\n\r\n");
        for (int i=1; i<size; i++) {
                debug_s(" %-10s---  %s\r\n",list[i].name, list[i].desc);
        }
        debug_s("\r\n");
}

/* shell reset */
void shell_reset(shell_param_t *param)
{
        shell_clear(param);
        __set_FAULTMASK(1);
        NVIC_SystemReset();
}
/* shell clear */
void shell_clear(shell_param_t *param)
{
        hw_print_str("\033[2J");
}

/* shell module list */
void shell_module_list(shell_param_t *param)
{
        debug_s("Support module list:\r\n\r\n");
        for (int i=0; i<shell_config.size; i++) {
                debug_s("-> %s\r\n", shell_list[i].name);
        }
        debug_s("\r\n");
}

/* shell module run entry */
void shell_run(uint8_t **argv, uint16_t argc)
{
        uint8_t id, cnt;
        shell_param_t paramt;

        id = shell_config.id;
        paramt.module = shell_list[id].name;
        paramt.size = shell_list[id].size;
        paramt.argv = argv;
        paramt.argc = argc;
        cnt = argc;

        /* recursive call the entry */
        while(--cnt > 0) {
                paramt.name = argv[cnt];
                paramt.index = cnt;
                shell_run_entry(shell_list[id].list, &paramt);
        }
}

void shell_cmd_main_entry(uint8_t **argv, uint16_t argc)
{
        uint8_t size;
        shell_param_t param;

        /* default shell command anaysis */
        size = shell_list[0].size;
        for (int i=0; i<size; i++) {
                if (shell_strcmp(shell_cmd[i].name, argv[0])) {
                        param.argc = argc;
                        param.argv = argv;
                        shell_cmd[i].entry(&param);
                        shell_config.id = 0;
                        return;
                }
        }

        size = shell_config.size;
        /* module command list anaysis */
        for (int i=0; i<size; i++) {
                if (shell_strcmp(shell_list[i].name, argv[0])) {
                        shell_config.id = i;
                        shell_run(argv, argc);
                        shell_config.id = 0;
                        return;
                }
        }
        debug_s("\r\nCommand '%s' is not find, you can try '?'\r\n\r\n", argv[0]);
}

void shell_cmd_search(uint8_t *src, uint8_t *tag, uint16_t *len)
{
#warning Not Implemented
}

