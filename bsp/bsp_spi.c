/*
 * SPI.c
 *
 *  Created on: 2018Äê8ÔÂ13ÈÕ
 *      Author: ggg
 */
#include <ti/drivers/SPI.h>

#include "Board.h"
#include "debug.h"


#define SPI_RATE    4000000
#define SPI_BUF_LEN 64


static SPI_Transaction transaction;
static SPI_Handle handle;

void SPI_bsp_init(uint8_t* rxbuf, uint8_t* txbuf)
{
    SPI_Params params;

    // Init SPI and specify non-default parameters
    SPI_Params_init(&params);
    params.bitRate             = SPI_RATE;
    params.frameFormat         = SPI_POL0_PHA1;
    params.mode                = SPI_MASTER;
    params.transferMode        = SPI_MODE_BLOCKING;
    params.transferCallbackFxn = NULL;
    // Configure the transaction
    transaction.count = SPI_BUF_LEN;
    transaction.txBuf = txbuf;
    transaction.rxBuf = rxbuf;
    // Open the SPI and initiate the first transfer
    handle = SPI_open(Board_SPI0, &params);
}

uint8_t SPI_check_status(void)
{
    return transaction.status==SPI_TRANSFER_STARTED;
}


bool SPI_bsp_send(void *buffer, uint16_t size)
{
    bool ret = false;
    if (SPI_check_status()){
        return ret;
    }

    if (size > SPI_BUF_LEN){
        return ret;
    }
    transaction.txBuf = buffer;
    transaction.count = size;
    ret = SPI_transfer(handle, &transaction);
    if (false==ret){
        SPI_transferCancel(handle);
        transaction.txBuf = buffer;
        transaction.count = size;
        ret = SPI_transfer(handle, &transaction);
    }

    return ret;
}


void SPI_bsp_cancle(void)
{
    SPI_transferCancel(handle);
}

void SPI_bsp_close(void)
{
    SPI_close(handle);
}
