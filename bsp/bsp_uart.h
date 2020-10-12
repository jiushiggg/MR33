/*
 * uart.h
 *
 *  Created on: 2018Äê2ÔÂ28ÈÕ
 *      Author: ggg
 */

#ifndef BSP_UART_H_
#define BSP_UART_H_
#include <stdint.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>

extern int32_t bsp_uart_read(void *buffer, uint16_t size);
extern int32_t bsp_uart_write(void *buffer, uint16_t size);
extern void bsp_uart_close(void);
extern void bsp_uart_init(void);
extern void bsp_uart_control(uint16_t cmd);


#endif /* BSP_UART_H_ */
