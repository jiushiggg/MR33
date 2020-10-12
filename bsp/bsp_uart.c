/*
 * uart.c
 *
 *  Created on: 2018Äê2ÔÂ28ÈÕ
 *      Author: ggg
 */

/* Driver Header files */

#include "bsp_uart.h"
#include "Board.h"


UART_Handle uart_handle;
extern UART_Callback uart_read_callback;
extern UART_Callback uart_write_callback;

void bsp_uart_init(uint16_t len)
{
    UART_Params uartParams;

    /* Call driver init functions */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode    = UART_DATA_BINARY;
    uartParams.readDataMode     = UART_DATA_BINARY;
    uartParams.readReturnMode   = UART_RETURN_FULL;
    uartParams.readMode         = UART_MODE_CALLBACK;
    uartParams.writeMode         = UART_MODE_CALLBACK;
    uartParams.readCallback     = uart_read_callback;
    uartParams.writeCallback     = uart_write_callback;
    uartParams.readEcho         = UART_ECHO_OFF;
    uartParams.baudRate         = 115200;

    uart_handle = UART_open(Board_UART0, &uartParams);
    if (uart_handle == NULL) {
        /* UART_open() failed */
        while (1);
    }
}


//callback mode read
int32_t bsp_uart_read(void *buffer, uint16_t size)
{
    return UART_read(uart_handle, buffer, size);
}

int32_t bsp_uart_write(void *buffer, uint16_t size)
{
    return UART_write(uart_handle, buffer, size);
}
void bsp_uart_close(void)
{
    UART_close(uart_handle);
}

void bsp_uart_control(uint16_t cmd)
{
    UART_control(uart_handle, cmd, NULL);
}

#ifdef UART_TEST
void uart_test(void)
{
    const char  echoPrompt[] = "Echoing characters:\r\n";
    char        input;
    UART_write(uart, echoPrompt, sizeof(echoPrompt));

    /* Loop forever echoing */
    while (1) {
        UART_read(uart, &input, 1);
        UART_write(uart, &input, 1);
    }
}
#endif
