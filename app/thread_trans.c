/*
 * thread_main.c
 *
 *  Created on: 2020Äê9ÔÂ23ÈÕ
 *      Author: gaolongfei
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Event.h>

#include <ti/drivers/utils/list.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/Error.h>

#include "thread_trans.h"
#include "debug.h"
#include "event.h"
#include "bsp_spi.h"
#include "bsp_uart.h"

#define ABORT_ID    0x1000

#define THREAD_SPI_PEND_TIME (5000*1000/Clock_tickPeriod)
#define BUFFER_LEN      4096

typedef enum _recv_em{
    RECV_HEAD = 0,
    RECV_DATA,
    RECV_ERR
}recv_em;


typedef enum _trans_buf_status{
    TRANS_BUF_USING   =   0,
    TRANS_BUF_IDLE,
}trans_buf_status;

typedef struct trans_struct {
    List_Elem elem;
    uint8_t * buf;
    trans_buf_status buf_status;
    uint8_t buf_total;
    uint8_t buf_index;
} trans_struct;


static void thread_transmit_init(void);

List_List trans_list;
trans_struct trans_buf[2];
uint8_t buffer[2][BUFFER_LEN];

Mailbox_Handle trans_mbox;
extern Mailbox_Handle rf_mbox;

#define UART_HEAD   (uint16_t)24
#define UART_MAX_DATA   (uint16_t)512
#define UART_BUF_LEN    (UART_HEAD+UART_MAX_DATA)
uint8_t uart_txbuf[UART_BUF_LEN];                  // send buffer
uint8_t uart_rxbuf[UART_BUF_LEN];               // Receive  buffer
static uint8_t uart_sync[4] = {0xf0, 0xf0, 0xf0, 0xf0};

void *thread_transmit(UArg arg)
{
//    uint16_t i, n=0;
//    while(1)
//    {
////        memset(txbuf, 0, sizeof(txbuf));
////        n = rand()%BUF_LEN;
////        if (n==0){
////            n++;
////        }
////        for (i=0; i<n; i++){
////            txbuf[i] = '0'+i;
////        }
//
//        //SPI_bsp_send(txbuf, BUF_LEN);
//        //Task_sleep(1000/10);
//        pinfo("dongle: %s", __FUNCTION__);
//        Task_sleep(3000000/10);
//    }

    uart_tsk_msg_t msg;
    TRACE();
    thread_transmit_init();
    semaphore_uart_init();
    while(1){
        if(true != Mailbox_pend(trans_mbox, &msg, /*BIOS_WAIT_FOREVER*/THREAD_SPI_PEND_TIME)) {
            //todo: if uart ok feed watchdog
            continue;
        }
        if (MSG_UART_CB == msg.type) {
            uart_head_st* head  = (uart_head_st*)msg.buf;
            switch(head->ctrl){
                case CTRL_ACK:                  //uplink task, elinker send ack to dongle
                    trans_uplink_handle(&msg);
                    break;
                case CTRL_MSG:                  //downlink task, elinker send message to dongle
                    trans_downlink_handle(&msg);
                    break;
                default:
                    break;
            }
        }else if(MSG_UPLINK == msg.type){
            trans_uplink_handle(&msg);          //uplink task, dongle send message to elinker
        }else if(MSG_TRANS_ACK == msg.type){
            trans_downlink_handle(&msg);        //downlink task, dongle send ack to elinker
        }else{
            //todo: err message type
        }

    }
}
typedef enum _up_handle{
    UPLINK_START,
    UPLINK_JUDGE,
    UPLINK_TRNAS,
    UPLINK_ERR,
    UPLINK_END
}up_handle;

typedef enum _down_handle{
    DOWNLINK_START,
    DOWNLINK_JUDGE,
    DOWNLINK_ACK,
    DOWNLINK_ERR,
    DOWNLINK_END
}down_handle;

void trans_uplink_handle(uart_tsk_msg_t* msg)
{
    static up_handle u_status = UPLINK_START;


    switch(u_status){
        case UPLINK_START:
            task_id = head->idx;
            if (trans_buf.total)

            break;
        case :
            break;
        default:
            break;
    }
}

void trans_downlink_handle(uart_tsk_msg_t* msg)
{
    static down_handle d_status = DOWNLINK_START;
    uart_head_st* msg_head  = (uart_head_st*)msg.buf;
    uart_head_st* head_addr  = NULL;
    uart_head_st* data_addr = NULL;
    uint8_t i = 0;
    uint16_t crc = 0;
    static uint16_t task_id = 0;
    while(){
        switch(d_status){
            case DOWNLINK_START:
                task_id = head_addr->idx;
                head_addr = ap_malloc(sizeof(uart_head_st));
                memcpy(head_addr, msg_head, sizeof(uart_head_st));

                for(i=0; i<2; i++){
                    if (TRANS_BUF_IDLE == trans_buf[i].buf_status){
                        break;
                    }
                }
                if (i == 2){
                    data_addr = ap_malloc(head_addr->len);
                }else {
                    data_addr = trans_buf[i].buf;
                }
                d_status = DOWNLINK_JUDGE;
                //no break;
            case DOWNLINK_JUDGE:
                if (0 != memcmp(head_addr->sync, uart_sync, sizeof(uart_sync))){
                    goto downlink_ack;
                }
                if (head_addr->len > UART_MAX_DATA){
                    goto downlink_ack;
                }
                crc = CRC16_CaculateStepByStep();
                if (crc != head_addr->crc){
                    goto downlink_ack;
                }
                if (task_id == ABORT_ID){
                    //todo: send message to rf thread, stop current task
                    //then rf send message to transmit thread
                    break;
                }
                if (task_id != head_addr->id){
                    goto downlink_ack;
                }

                //break;
            case DOWNLINK_ACK:

                if (head_addr->pkg == head_addr->idx){
                    d_status = DOWNLINK_END;
                }
                break;
            case  DOWNLINK_END:
                ap_free(head_addr);
                ap_free(data_addr);
                d_status = DOWNLINK_START;
                break;
            default:
                break;
        }
    }

}

static List_Elem* trans_list_init(void)
{
    trans_buf[0].buf = buffer[0];
    trans_buf[1].buf = buffer[1];
    trans_buf[0].buf_total = trans_buf[1].buf_total = BUFFER_LEN/UART_MAX_DATA;
    trans_buf[0].buf_index = trans_buf[1].buf_index = 0;
    trans_buf[0].buf_status = trans_buf[1].buf_status = TRANS_BUF_IDLE;

    List_clearList(&trans_list);
    List_put(&trans_list, (List_Elem *)&trans_buf[0]);
    List_put(&trans_list, (List_Elem *)&trans_buf[1]);
    trans_list.tail->next = (List_Elem *)&trans_buf[0];

    return List_head(&trans_list);
}

static void thread_transmit_init(void)
{
    //init mailbox from spi call back function to spi msg handler task
    Mailbox_Params mboxParams;
    Error_Block eb;
    Error_init(&eb);
    Mailbox_Params_init(&mboxParams);
    trans_mbox = Mailbox_create(sizeof(uart_tsk_msg_t), 3, &mboxParams, &eb);
    if (trans_mbox == NULL)
        pinfo("Mailbox create failed");

    trans_list_init();
}

void uart_read_callback(UART_Handle handle, void *rxBuf, size_t size)
{
    static recv_em recv_status = RECV_HEAD;
    UARTCC26XX_Object *object =  handle->object;

    if (UART_OK == object->status) {
        if (RECV_HEAD == recv_status) {
            recv_status = RECV_DATA;
            if (0 == ((uart_head_st*)rxBuf)->len){
                recv_status = RECV_HEAD;
            }else {
                recv_status = RECV_DATA;
            }
            UART_read(handle, uart_rxbuf+sizeof(uart_head_st), ((uart_head_st*)rxBuf)->len);
        } else if (RECV_DATA == recv_status) {
            UART_read(handle, uart_rxbuf, sizeof(uart_head_st));
            recv_status = RECV_HEAD;
            //todo: post message to thread_transmit()
        } else {
            UART_read(handle, uart_rxbuf, sizeof(uart_head_st));
            recv_status = RECV_HEAD;
        }
    } else {
        if (RECV_HEAD == recv_status) {
            recv_status = RECV_HEAD;
            //todo: post err message to thread_transmit()
        } else if (RECV_DATA == recv_status) {
            if (size == ((uart_head_st*)uart_rxbuf)->len){
                //todo: post message to thread_transmit()
            } else {
                //todo: post err message to thread_transmit()
            }
            recv_status = RECV_HEAD;
        } else {
            recv_status = RECV_HEAD;
            //todo: post err message to thread_transmit()
        }
        UART_control(handle, UARTCC26XX_CMD_RX_FIFO_FLUSH, NULL);
        UART_read(handle, uart_rxbuf, sizeof(uart_head_st));
    }
}


void uart_write_callback(UART_Handle handle, void *rxBuf, size_t size)
{
    uart_write_post();
}
