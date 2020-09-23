/*
 * uart.c
 *
 *  Created on: 2018��2��28��
 *      Author: ggg
 */

/* Driver Header files */


#include <ti/drivers/uart/UARTCC26XX.h>
#include "sys_cfg.h"
#include "uart.h"
#include "Board.h"
#include "protocol.h"


UART_Handle uart_handle;

extern void readCallback(UART_Handle handle, void *rxBuf, size_t size);


void UART_appInit(void)
{

    UART_Params uartParams;
    /* Call driver init functions */

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readMode = UART_MODE_CALLBACK;
    uartParams.readEcho = UART_ECHO_OFF;
    //uartParams.baudRate = 115200;
    uartParams.baudRate = 1500000;
    uartParams.readCallback  = readCallback;

    uart_handle = UART_open(Board_UART0, &uartParams);

    if (uart_handle == NULL) {
        /* UART_open() failed */
        while (1);
    }

    UART_control(uart_handle, UARTCC26XX_CMD_RETURN_PARTIAL_ENABLE, NULL);
    /* Loop forever echoing */

    UART_read(uart_handle, recv_once_buf, sizeof(recv_once_buf));

}


//callback mode read
int32_t UART_appRead(void *buffer, size_t size)
{
    return UART_read(uart_handle, buffer, size);
}

int32_t UART_appWrite(void *buffer, size_t size)
{
    return UART_write(uart_handle, buffer, size);
}

uint8_t UART_checkStatus(void)
{
	return 1;
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
