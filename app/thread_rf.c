/*
 * thread_rf.c
 *
 *  Created on: 2020Äê9ÔÂ3ÈÕ
 *      Author: gaolongfei
 */

#include <stdint.h>
#include <stddef.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Event.h>

#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/Error.h>

#include "Board.h"
#include "thread_rf.h"
#include "debug.h"

Mailbox_Handle rf_mbox;

#define RF_MBOX_PEND_TIME (10000000/Clock_tickPeriod)

static void thread_rf_init(void);

#define BUF_LEN 64
uint8_t txbuf1[BUF_LEN];                  // Receive and transmit buffer

Void *thread_rf(UArg arg0, UArg arg1)
{
//    uint16_t i, n=0;
//    while(1)
//    {
//        memset(txbuf1, 0, sizeof(txbuf1));
//        n = rand()%BUF_LEN;
//        if (n==0){
//            n++;
//        }
//        for (i=0; i<n; i++){
//            txbuf1[i] = '0'+2;
//        }
//
//        SPI_bsp_send(txbuf1, BUF_LEN);
//        Task_sleep(1000000/10);
//    }

    TRACE();

    thread_rf_init();
    while(1){
        pinfo("dongle: %s", __FUNCTION__);
        //TRACE();
        Task_sleep(100000/10);
    }
}

static void thread_rf_init(void)
{
    //init mailbox from spi call back function to spi msg handler task
    Mailbox_Params mboxParams;
    Error_Block eb;
    Error_init(&eb);
    Mailbox_Params_init(&mboxParams);

    //init mailbox from rf handler thread
    rf_mbox = Mailbox_create(sizeof(rf_tsk_msg_t), 2, &mboxParams, &eb);
    if (rf_mbox == NULL)
        pinfo("Mailbox create failed");


    //ap_flash_init();
}
