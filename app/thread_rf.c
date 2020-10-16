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
#include "trans_struct.h"

typedef struct _rf_tsk_msg {
    uint16_t type;
    uint16_t id;
    uint32_t len;
    uint8_t* buf;
    uint32_t size;
    void* extra;
}rf_tsk_msg_t;


Mailbox_Handle rf_mbox;

#define RF_MBOX_PEND_TIME (10000000/Clock_tickPeriod)

static void thread_rf_init(void);


int8_t forward_msg_rfthread(uint16_t id, uint8_t* data, uint32_t length, uint32_t size, uint32_t storage)
{
    rf_tsk_msg_t msg = {
        .type = MSG_UPLINK_REQ,
        .id = id,
        .len = length,
        .buf = data,
        .size = size,
        .extra = (void*)storage
    };

    //send mailbox to msg handler task
    if(true == Mailbox_post(rf_mbox, &msg, BIOS_NO_WAIT))
        return 0;

//   bsp_spi_downlink_data_free(data, size, storage);
//todo: free data
    pinfo("%s mail box post error\r\n", __func__);
    return -1;
}


Void *thread_rf(UArg arg0, UArg arg1)
{
    TRACE();
    thread_rf_init();
    rf_tsk_msg_t msg;

    while(1){
        if(TRUE != Mailbox_pend(rf_mbox, &msg, /*BIOS_WAIT_FOREVER*/RF_MBOX_PEND_TIME)) {
            continue;
        }
        pinfo("%s receive Mailbox, ", __func__);
        pinfo("type: %d, id: %x, len: %d", msg.type, msg.id, msg.len);
        rf_handle(&msg);
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
