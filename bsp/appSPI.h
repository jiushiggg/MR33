/*
 * mySPI.h
 *
 *  Created on: 2018��8��13��
 *      Author: ggg
 */

#ifndef BSP_APPSPI_H_
#define BSP_APPSPI_H_
#include <stdint.h>
#include <ti/drivers/SPI.h>

extern void SPI_appInit(uint8_t* rxbuf, uint8_t* txbuf);
extern bool SPI_appRecv(void *buffer, uint16_t size);
extern bool SPI_appSend(void *buffer, uint16_t size);
extern void SPI_bufferInit(uint8_t* rxbuf, uint8_t* txbuf);
extern void SPI_cancle(void);
extern uint8_t SPI_checkStatus(void);
extern void SPI_preSend(void);

extern SPI_Handle handle;
extern SPI_Transaction transaction;

#endif /* BSP_APPSPI_H_ */
