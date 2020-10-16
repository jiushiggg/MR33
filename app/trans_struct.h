/*
 * trans_struct.h
 *
 *  Created on: 2020Äê10ÔÂ15ÈÕ
 *      Author: gaolongfei
 */

#ifndef APP_TRANS_STRUCT_H_
#define APP_TRANS_STRUCT_H_


typedef enum _recv_em{
    RECV_HEAD = 0,
    RECV_DATA,
    RECV_ERR
}recv_em;

typedef enum _em_msg_type{
    MSG_UPLINK_REQ = (uint8_t)1,
    MSG_UART_CB,
    MSG_TRANS_ACK,
    MSG_EVENT,
    MSG_ERR
}em_msg_type;

typedef enum _em_rf_task{
    RF_UPDATE = (uint8_t)1,
    RF_HB,
    RF_BG_SCAN,
    RF_AP_LOCATION,
    RF_SURVEY,          //5
    RF_HB_BG,
    RF_UPLINK,
    RF_IDLE,
}em_rf_task;

typedef  struct _uart_tsk_msg_t{
    em_msg_type type;
    em_rf_task rf_task;
    uint16_t id;
    uint32_t len;
    uint8_t* buf;
    uint32_t size;
    void* extra;
}uart_tsk_msg_t;

typedef enum _trans_buf_status{
    TRANS_BUF_USING   =   0,
    TRANS_BUF_IDLE,
}trans_buf_status;




typedef enum _uart_head_ctrl{
    CTRL_ACK = (uint8_t)1,
    CTRL_MSG = (uint8_t)2,
    CTRL_ERR_LEN = (uint8_t)0x20,
    CTRL_ERR_CRC = (uint8_t)0x40,
    CTRL_ERR_ID =  (uint8_t)0x80,
    CTRL_ERR_HEAD = (uint8_t)0x60,
    CTRL_OVER_FLOW = (uint8_t)0xC0,
}uart_head_ctrl;

typedef struct _uart_head_st{
    uint8_t sync[4];
    uint16_t tx_sn;
    uint16_t rx_sn;
    uart_head_ctrl ctrl;
    uint8_t win;
    uint16_t id;
    uint16_t pkg;
    uint16_t idx;
    uint16_t ack_req;
    uint16_t next_id;
    uint16_t len;
    uint16_t crc;
    uint8_t data[0];
}uart_head_st;


#endif /* APP_TRANS_STRUCT_H_ */
