/*
 * bsp_spi.h
 *
 *  Created on: 2020Äê9ÔÂ24ÈÕ
 *      Author: gaolongfei
 */

#ifndef BSP_BSP_SPI_H_
#define BSP_BSP_SPI_H_
#include <stdbool.h>

void SPI_bsp_init(uint8_t* rxbuf, uint8_t* txbuf);
uint8_t SPI_check_status(void);
bool SPI_bsp_send(void *buffer, uint16_t size);
void SPI_bsp_cancle(void);
void SPI_bsp_close(void);

#endif /* BSP_BSP_SPI_H_ */
