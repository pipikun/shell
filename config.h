#ifndef config_H__
#define config_H__

/* the on or off value*/
#define ON                                      1
#define OFF                                     0

/* uart config */
#define UART_DEBUG_HIM                          huart2
#define UART_DEBUG                              UART2
#define UART_DMA_RX                             hdma_usart2_rx

#define SHELL_MODE				ON
#define SHELL_DEBUG_I				ON
#define SHELL_DEBUG_S				ON
#define SHELL_DEBUG_E				ON

/* I2C config  */
#define I2C_SDA_GPIO                            GPIOC
#define I2C_SDA_PIN                             GPIO_PIN_2
#define I2C_SCL_GPIO                            GPIOC
#define I2C_SCL_PIN                             GPIO_PIN_3

#define __I2C_DEFAULT_VAL                       0x0U

/* MDIO config */
#define MDIO_GPIO                               GPIOC
#define MDIO_PIN                                GPIO_PIN_8

#define MDC_GPIO                                GPIOC
#define MDC_PIN                                 GPIO_PIN_9

/* shell config */
#if SHELL_MODE

#define SHELL_BUF_SIZE                          256
#define SHELL_CMD_SIZE                          20
#define SHELL_HISTORY_SIZE                      10
#define SHELL_SPLIT_CHAR                        " "
#define USER_LOCAL                              "root"
#define USER_SPLT                               "@"
#define USER_NAME                               "shell"
#define USER_END                                ":"
#define USER_BORD                               "STM32F1xx"
#define USER_BORD_ID                            "001"

#define USER_INFO                               (char *)(USER_LOCAL USER_SPLT USER_NAME \
                                                USER_END)

#endif


#if SHELL_DEBUG_I
        #define debug_i(fmt, ...)               printf(fmt, ##__VA_ARGS__)
	#define debug_iln(fmt, ...)		printf(fmt, ##__VA_ARGS__)
#else
        #define debug_i(fmt, ...)
	#define debug_iln(fmt, ...)
#endif

#if SHELL_DEBUG_S
        #define debug_s(fmt, ...)               printf(fmt, ##__VA_ARGS__)
	#define debug_sln(fmt, ...)		printf(fmt, ##__VA_ARGS__)
#else
        #define debug_s(fmt, ...)
	#define debug_sln(fmt, ...)
#endif

#if SHELL_DEBUG_E
        #define debug_d(fmt, ...)               printf(fmt, ##__VA_ARGS__)
	#define debug_dln(fmt, ...)             printf(fmt, ##__VA_ARGS__)
#else
        #define debug_d(fmt, ...)
	#define debug_dln(fmt, ...)
#endif

#define	KEY_UP					'A'
#define KEY_DOWN				'B'
#define KEY_RIGHT				'C'
#define KEY_LEFT				'D'

#endif
