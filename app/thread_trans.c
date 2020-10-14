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
#include "memery.h"
#include "debug.h"
#include "event.h"
#include "bsp_spi.h"
#include "bsp_uart.h"
#include "crc16.h"
#include "cc2640r2_rf.h"

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
static void trans_downlink_handle(uart_tsk_msg_t* msg);
static void trans_uplink_handle(uart_tsk_msg_t* msg);

#define UART_BUF_NUM    2
#define APP_BUF_NUM     2
List_List trans_list;
trans_struct trans_buf[APP_BUF_NUM];
uint8_t buffer[APP_BUF_NUM][BUFFER_LEN];

Mailbox_Handle trans_mbox;
extern Mailbox_Handle rf_mbox;

#define UART_HEAD   (uint16_t)24
#define UART_MAX_LEN   (uint16_t)512
#define UART_BUF_LEN    (UART_HEAD+UART_MAX_LEN)

uint8_t uart_rxbuf[UART_BUF_NUM][UART_BUF_LEN];               // Receive  buffer
const uint8_t uart_sync[4] = {0xf0, 0xf0, 0xf0, 0xf0};
int16_t rf_ch;

void *thread_transmit(UArg arg)
{
    uart_tsk_msg_t msg;
    TRACE();
    thread_transmit_init();
    bsp_uart_init();
    semaphore_uart_init();
    bsp_uart_read(uart_rxbuf[0], sizeof(uart_head_st));
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
    DOWNLINK_START  = (uint8_t)1,
    DOWNLINK_JUDGE,
    DOWNLINK_PACKAGE,
    DOWNLINK_SEND_ACK,
    DOWNLINK_ERR,
    DOWNLINK_END
}down_handle;

void trans_uplink_handle(uart_tsk_msg_t* msg)
{
#if 0
    static up_handle u_status = UPLINK_START;
    static uart_head_st* head_addr  = NULL;
    static uart_head_st* data_addr  = NULL;
    static uart_head_st* tx_head_addr = NULL;
    uint8_t i, exit_flg = 1;

    while(exit_flg){
        switch(u_status){
            case UPLINK_START:
                break;
            case UPLINK_TRNAS:
                break;
            default:
                break;
        }
    }
#endif
}

void trans_downlink_handle(uart_tsk_msg_t* msg)
{
    static down_handle d_status = DOWNLINK_START;
    static uart_head_st* head_addr  = NULL;
    static uart_head_st* data_addr  = NULL;
    static uart_head_st* tx_head_addr = NULL;
    uart_head_st* msg_head  = (uart_head_st*)msg->buf;
    uint8_t exit_flg = 1;
    static uint16_t task_id = 0;
    static uint8_t i;

    while(exit_flg){
        switch(d_status){
            case DOWNLINK_START:
                task_id = msg_head->idx;
                head_addr = ap_malloc(sizeof(uart_head_st));
                tx_head_addr = ap_malloc(sizeof(uart_head_st));

                for(i=0; i<APP_BUF_NUM; i++){
                    if (TRANS_BUF_IDLE == trans_buf[i].buf_status){
                        break;
                    }
                }
                if (i == APP_BUF_NUM){
                    data_addr = ap_malloc(UART_MAX_LEN);
                }else {
                    trans_buf[i].buf_status = TRANS_BUF_USING;
                    data_addr = (uart_head_st*)trans_buf[i].buf;
                }
                d_status = DOWNLINK_JUDGE;
                break;
            case DOWNLINK_JUDGE:
            {
                uint16_t crc = 0;

                memcpy(head_addr, msg_head, sizeof(uart_head_st));
                memcpy(data_addr, msg_head->data, head_addr->len);

                if (0 != memcmp(head_addr->sync, uart_sync, sizeof(uart_sync))){
                    tx_head_addr->ctrl = CTRL_ERR_HEAD;
                    d_status = DOWNLINK_PACKAGE;
                    break;
                }
                if (head_addr->len > UART_MAX_LEN){
                    tx_head_addr->ctrl = CTRL_ERR_LEN;
                    d_status = DOWNLINK_PACKAGE;
                    break;
                }
                crc = CRC16_CaculateStepByStep(0, (uint8_t*)head_addr, offsetof(uart_head_st, crc));
                crc = CRC16_CaculateStepByStep(crc, (uint8_t*)data_addr, head_addr->len);
                if (crc != head_addr->crc){
                    tx_head_addr->ctrl = CTRL_ERR_CRC;
                    d_status = DOWNLINK_PACKAGE;
                    break;
                }
                if (head_addr->id == ABORT_ID){
                    RF_cancle(rf_ch);
                    //todo: send message to rf thread, stop current rf task
                    //then rf send message to transmit thread
                    exit_flg = 0;
                    d_status = DOWNLINK_SEND_ACK;
                    break;
                }
                if (task_id != head_addr->id){
                    tx_head_addr->ctrl = CTRL_ERR_ID;
                    d_status = DOWNLINK_PACKAGE;
                    break;
                }
                if (APP_BUF_NUM == i){
                    //todo: if return no buffer, need a event, check buffer, then send message to elinker
                    d_status = DOWNLINK_PACKAGE;
                    break;
                }
                if ((uint8_t*)data_addr+head_addr->len > trans_buf[i].buf){
                    tx_head_addr->ctrl = CTRL_OVER_FLOW;
                    d_status = DOWNLINK_PACKAGE;
                    break;
                }
                data_addr += head_addr->len;
                d_status = DOWNLINK_PACKAGE;
                break;
            }
            case DOWNLINK_PACKAGE:
                memcpy(tx_head_addr, uart_sync, sizeof(uart_sync));
                tx_head_addr->tx_sn++;
                tx_head_addr->rx_sn = head_addr->tx_sn;
                tx_head_addr->ctrl |= CTRL_ACK;
                if (APP_BUF_NUM == i){
                    tx_head_addr->win = 0;
                } else {
                    tx_head_addr->win = (&trans_buf[i].buf[BUFFER_LEN]-(uint8_t*)data_addr)/UART_MAX_LEN;
                }
                tx_head_addr->id = head_addr->id;
                tx_head_addr->ack_req = 0;
                tx_head_addr->next_id = 0;
                tx_head_addr->len = 0;
                tx_head_addr->crc = CRC16_CaculateStepByStep(0, (uint8_t*)tx_head_addr, offsetof(uart_head_st, crc));
                //todo: add send dongle log
                d_status = DOWNLINK_SEND_ACK;
                break;
            case DOWNLINK_SEND_ACK:
            {
                uart_write_pend(EVENT_WAIT_FOREVER);
                bsp_uart_write(tx_head_addr, sizeof(uart_head_st));

                if (head_addr->pkg == head_addr->idx){
                    d_status = DOWNLINK_END;
                }else {
                    d_status = DOWNLINK_JUDGE;
                }
                break;
            }
            case  DOWNLINK_END:
                ap_free(head_addr, sizeof(uart_head_st));
                head_addr = NULL;
                ap_free(data_addr, UART_MAX_LEN);
                data_addr = NULL;
                ap_free(tx_head_addr, sizeof(uart_head_st));
                tx_head_addr = NULL;
                exit_flg = 0;
                d_status = DOWNLINK_START;
                break;
            case  DOWNLINK_ERR: //no break;
            default:
                //todo:error handle
                break;
        }
    }
}

static List_Elem* trans_list_init(void)
{
    trans_buf[0].buf = buffer[0];
    trans_buf[1].buf = buffer[1];
    trans_buf[0].buf_total = trans_buf[1].buf_total = BUFFER_LEN/UART_MAX_LEN;
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

static recv_em recv_status = RECV_HEAD;
static uint16_t wantedbytes = sizeof(uart_head_st);
static uint8_t uart_buf_idx = 0;
//todo: add a timer to control read time.
void uart_read_callback(UART_Handle handle, void *rxBuf, size_t size)
{
    UARTCC26XX_Object *object =  handle->object;

    if (UART_OK == object->status) {
        if (RECV_HEAD == recv_status) {
            if (size==sizeof(uart_head_st) && 0==((uart_head_st*)rxBuf)->len){
                recv_status = RECV_HEAD;
                wantedbytes = sizeof(uart_head_st);

                uart_tsk_msg_t msg = {
                    .type = MSG_UART_CB,
                    .buf = uart_rxbuf[uart_buf_idx]
                };
                Mailbox_post(trans_mbox, &msg, BIOS_NO_WAIT);
                uart_buf_idx = (++uart_buf_idx) >= 2 ? 0 : uart_buf_idx;
            }else if (size == sizeof(uart_head_st)){
                recv_status = RECV_DATA;
                wantedbytes = ((uart_head_st*)rxBuf)->len > UART_MAX_LEN ? UART_MAX_LEN : ((uart_head_st*)rxBuf)->len;
            }else {
                recv_status = RECV_HEAD;
                wantedbytes = sizeof(uart_head_st);
            }

            UART_read(handle, uart_rxbuf[uart_buf_idx]+sizeof(uart_head_st), wantedbytes);
        } else if (RECV_DATA==recv_status && wantedbytes == size) {
            recv_status = RECV_HEAD;

            uart_tsk_msg_t msg = {
                .type = MSG_UART_CB,
                .buf = uart_rxbuf[uart_buf_idx]
            };
            Mailbox_post(trans_mbox, &msg, BIOS_NO_WAIT);
            uart_buf_idx = (++uart_buf_idx) >= 2 ? 0 : uart_buf_idx;
            UART_read(handle, uart_rxbuf[uart_buf_idx], sizeof(uart_head_st));
        } else {
            UART_read(handle, uart_rxbuf[uart_buf_idx], sizeof(uart_head_st));
            recv_status = RECV_HEAD;
        }
    } else {
        if (RECV_DATA==recv_status && wantedbytes == size) {
            uart_tsk_msg_t msg = {
                .type = MSG_UART_CB,
                .buf = uart_rxbuf[uart_buf_idx]
            };
            Mailbox_post(trans_mbox, &msg, BIOS_NO_WAIT);
            uart_buf_idx = (++uart_buf_idx) >= 2 ? 0 : uart_buf_idx;
        }
        recv_status = RECV_HEAD;
        UART_control(handle, UARTCC26XX_CMD_RX_FIFO_FLUSH, NULL);
        UART_read(handle, uart_rxbuf[uart_buf_idx], sizeof(uart_head_st));
    }
}


void uart_write_callback(UART_Handle handle, void *rxBuf, size_t size)
{
    uart_write_post();
}
