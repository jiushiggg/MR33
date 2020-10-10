/*
 * thread_main.h
 *
 *  Created on: 2020Äê9ÔÂ23ÈÕ
 *      Author: gaolongfei
 */

#ifndef APP_THREAD_TRANS_H_
#define APP_THREAD_TRANS_H_


typedef enum _uart_head_ctrl{
    CTRL_ACK = 0,
    CTRL_MSG,
}uart_head_ctrl;

typedef struct _uart_head_st{
    uint8_t sync[4];
    uint16_t tx_sn;
    uint16_t rx_sn;
    uart_head_ctrl ctrl;
    uint8_t win;
    uint8_t id;
    uint16_t pkg;
    uint16_t idx;
    uint32_t ret_code;
    uint16_t len;
    uint16_t crc;
    uint8_t data[0];
}uart_head_st;


typedef  struct _uart_tsk_msg_t{
    uint16_t type;
    uint16_t id;
    uint32_t len;
    uint8_t* buf;
    uint32_t size;
    void* extra;
}uart_tsk_msg_t;

extern void *thread_transmit(UArg arg);

#endif /* APP_THREAD_TRANS_H_ */
