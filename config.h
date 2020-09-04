#ifndef config_H__
#define config_H__

/* the on or off value*/
#define ON                                      1
#define OFF                                     0

/* uart config */
#define UART_DEBUG_HIM                          huart2
#define UART_DEBUG                              UART2

#define shell_mode				ON
#define shell_debug_i				ON
#define shell_debug_s				ON
#define shell_debug_d				ON

/* shell config */
#if shell_mode
        #define SHELL_BUF_SIZE                  256
        #define SHELL_CMD_SIZE                  256
        #define USER_LOCAL                      "root"
        #define USER_SPLT                       "@"
        #define USER_NAME                       "carson"
        #define USER_END                        ":"
        #define USER_BORD                       "STM32F1xx"
        #define USER_BORD_ID                    "001"
        #define USER_INFO(u1,u2,u3,u4)          u1##u2##u3##u4
#endif

#if  shell_debug_i
        #define debug_i(fmt, ...)               printf(fmt, ##__VA_ARGS__)
	#define debug_iln(fmt, ...)		printf(fmt, ##__VA_ARGS__)
#else
        #define debug_i(fmt, ...)
	#define debug_iln(fmt, ...)
#endif

#if  shell_debug_s
        #define debug_s(fmt, ...)               printf(fmt, ##__VA_ARGS__)
	#define debug_sln(fmt, ...)		printf(fmt, ##__VA_ARGS__)
#else
        #define debug_s(fmt, ...) 
	#define debug_sln(fmt, ...)
#endif

#if  shell_debug_d
        #define debug_d(fmt, ...)               printf(fmt, ##__VA_ARGS__)
	#define debug_dln(fmt, ...)             printf(fmt, ##__VA_ARGS__)
#else
        #define debug_d(fmt, ...) 
	#define debug_dln(fmt, ...)
#endif


#endif
