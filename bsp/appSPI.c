/*
 * SPI.c
 *
 *  Created on: 2018��8��13��
 *      Author: ggg
 */
#include <ti/drivers/GPIO.h>
//#include <ti/drivers/spi/SPICC26XXDMA.h>

#include "Board.h"
//#include "sys_cfg.h"
#include "appSPI.h"
#include "SPI_private.h"
#include "debug.h"


#define SPI_RATE    4000000
#define SPI_BUF_LEN SPIPRIVATE_LEN_ALL

extern void transferCallback(SPI_Handle handle, SPI_Transaction *transaction);
SPI_Transaction transaction;
SPI_Handle handle;

void SPI_appInit(uint8_t* rxbuf, uint8_t* txbuf)
{
    SPI_Params params;

    // Init SPI and specify non-default parameters
    SPI_Params_init(&params);
    params.bitRate             = SPI_RATE;
    params.frameFormat         = SPI_POL0_PHA0;
    params.mode                = SPI_SLAVE;
    params.transferMode        = SPI_MODE_CALLBACK;
    params.transferCallbackFxn = transferCallback;
    // Configure the transaction
    transaction.count = SPI_BUF_LEN;
    transaction.txBuf = txbuf;
    transaction.rxBuf = rxbuf;
    // Open the SPI and initiate the first transfer
    pinfo("spi init len:%d", transaction.count);
    handle = SPI_open(Board_SPI1, &params);
//    SPI_control(handle, SPICC26XXDMA_RETURN_PARTIAL_ENABLE, NULL);
    SPI_transfer(handle, &transaction);
}

uint8_t SPI_checkStatus(void)
{
	return transaction.status==SPI_TRANSFER_STARTED;
}

void SPI_preSend(void)
{
	GPIO_write(Board_SPI_SLAVE_READY, 1);
}

bool SPI_appSend(void *buffer, uint16_t size)
{
	bool ret = false;
	if (SPI_checkStatus()){
		return ret;
	}
//    transaction.txBuf = buffer;
	transaction.count = size;
	ret = SPI_transfer(handle, &transaction);
	if (false==ret && SPI_TRANSFER_STARTED!=transaction.status){
		pinfo("Pre-setS:%d\r\n",transaction.status);
		SPI_transferCancel(handle);
	    transaction.count = SPI_BUF_LEN;
		ret = SPI_transfer(handle, &transaction);
		pinfo("resetS:%d\r\n",transaction.status);
	}
	GPIO_write(Board_SPI_SLAVE_READY, 0);

    return ret;
}

bool SPI_appRecv(void *buffer, uint16_t size)
{
	bool ret = false;

	if (SPI_checkStatus()){
		return ret;
	}

	GPIO_write(Board_SPI_SLAVE_READY, 1);
//    transaction.rxBuf = buffer;
	transaction.count = size;
	ret = SPI_transfer(handle, &transaction);
	if (false==ret && SPI_TRANSFER_STARTED!=transaction.status){
		pinfo("Pre-setR:%d\r\n",transaction.status);
		SPI_transferCancel(handle);
	    transaction.count = SPI_BUF_LEN;
		ret = SPI_transfer(handle, &transaction);
		pinfo("resetR:%d\r\n",transaction.status);
	}
    return ret;
}

void SPI_cancle(void)
{

    SPI_transferCancel(handle);

}


void SPI_bufferInit(uint8_t* rxbuf, uint8_t* txbuf)
{
    transaction.txBuf = txbuf;
    transaction.rxBuf = rxbuf;
}
