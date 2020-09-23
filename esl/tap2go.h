/*
 * tap2go.h
 *
 *  Created on: 2018年11月8日
 *      Author: ggg
 */

#ifndef ESL_TAP2GO_H_
#define ESL_TAP2GO_H_

#include "tap2go_event.h"

#define ID_LEN  4

#define T2G_FRAME1_CTRL                 (0X70|0X01)
#define UPLINK_DATA_CTRL                (0X90|0X01)
#define T2G_DATA_CTRL                   (0X90|0X08)
#define T2G_QUERY_CTRL                  (0x90|0X02)
#define T2G_COMPLETE_CTRL               (0x90|0X04)
#define CTRL_MASK                       (0xFF)

#define     T2G_RESPONSE_CTRL           (0X90|0X03)
#define     T2G_HANDSHAKE_CTRL          (0X70|0X01)


#define UPLINK_T2G_VERIFY      0x42
#define UPLINK_T2G_MODE     0x40
#define UPLINK_MASS_DATA    0x41

#define UPLINK_GEOLOCATION_HB  0x50

#define UPLINK_BINDING_INT	0X81
#define UPLINK_BINDING_STR	0X82
#define UPLINK_UPDATE_INT	0X83
#define UPLINK_UPDATE_STR	0X84


//Bit0:100k，bit1:125k，bit2:500k，bit3:1M，bit6:1.5M，bit7:2M

typedef enum {
    RATE_100K = 1,
    RATE_125K = 2,
    RATE_500K = 4,
    RATE_1M = 8,
    RATE_1D5M = 0x40,
    RATE_2M = (uint8_t)0x80
}em_rate_bitmap;

typedef enum {
    POWER_0 = 0,
    POWER_6 = 1,
    POWER_10 = 2,
    POWER_13 = (uint8_t)4,
}em_power_bitmap;

//ESL: bit7:-29db bit6:-28db bit5:5db bit4:4db bit3:3db bit2:2db bit1:1db bit0:0db
typedef enum {
    ESL_POWER_0 = 0,
    ESL_POWER_1 = 1,
    ESL_POWER_2 = 2,
    ESL_POWER_3 = 4,
    ESL_POWER_4 = 0X10,
    ESL_POWER_5 = (uint8_t)0X20,
}em_esl_power_bitmap;

#pragma pack(1)
typedef struct st_req_content{
    em_rate_bitmap rate_bitmap;
    em_power_bitmap power_bitmap;
    uint16_t RF_FIFO;
    uint16_t data_len;
    uint8_t ap_id;
    uint8_t reserved[9];
}st_req_content;

typedef struct st_t2g_up_req{
    uint8_t ctrl;
    uint8_t id[ID_LEN];
    uint8_t session_id;
    uint8_t default_ch;
    uint8_t req_type;
    st_req_content req;
    uint8_t ap_ch;
    uint8_t ap_id;
    uint16_t default_rate;          //握手时的信道
    uint8_t default_power;          //
    uint8_t status;
    uint16_t timeout;
    uint8_t recv_info_ch;
}st_t2g_up_req;

typedef struct st_t2g_down_rsp{
    uint8_t ctrl;
    uint8_t id[ID_LEN];
    uint8_t session_id;
    uint8_t status;
    uint8_t ap_ch;
    uint8_t ap_id;
    uint8_t esl_uplink_ch;
    uint8_t rate_bitmap;
    uint8_t power_bitmap;
    uint16_t RF_FIFO;
    uint8_t reserve[10];
    uint16_t crc;
}st_t2g_down_rsp;

typedef struct st_t2g_frame1{
    uint8_t ctrl;
    uint8_t id[ID_LEN];
    uint8_t session_id;
    uint8_t default_ch;
    uint8_t req_type;
    st_req_content req;
    uint16_t crc;
}st_t2g_frame1;

typedef struct st_t2g_data{
    uint8_t ctrl;
    uint8_t sid;
    uint16_t package_num;
    uint8_t flg;
    uint8_t rx_ack_flg;
    uint8_t task_number;
    uint8_t data[121];
}st_t2g_data;


typedef struct st_uplink_data{
    uint8_t ctrl;
    uint8_t sid;
    uint16_t package_num;
    uint8_t flg;
    uint8_t data[27];
    uint16_t crc;
}st_uplink_data;

typedef struct st_t2g_query{
    uint8_t ctrl;
    uint8_t sid;
    uint8_t slot;
    uint16_t total_send;
    uint8_t reserved;
    uint8_t unused[26];
    uint16_t crc;
}st_t2g_query;

typedef struct st_t2g_complete{
    uint8_t ctrl;
    uint8_t sid;
    uint8_t slot;
    uint16_t total_send;
    uint8_t ap_id;
    uint8_t unused[26];
    uint16_t crc;
}st_t2g_complete;

typedef struct st_t2g_ack{
    uint8_t ctrl;
    uint8_t sid;
    uint8_t slot;
    uint16_t loss_num;
    uint8_t reserve;
    uint8_t lost_packet[26];
    uint16_t crc;
}st_t2g_ack;

typedef enum T2G_cmd{
    T2G_FSM_INIT             = 0X01,

    T2G_FSM_RX_FRAME1_CONFIG = 0X02,
    T2G_FSM_RX_FRAME1        = 0X04,
    T2G_FSM_RX_FRAME1_HANDLE = 0X08,

    T2G_FSM_RX_DATA_CONFIG   = 0X10,
    T2G_FSM_RX_DATA          = 0X20,
    T2G_FSM_RX_DATA_HANDLE   = 0X40,

    T2G_FSM_TX_DATA_CONFIG   = 0X80,
    T2G_FSM_TX_DATA          = 0X100,
    T2G_FSM_TX_DATA_HANDLE    = 0X200,

    T2G_FSM_FINISH               = 0X400,
    T2G_FSM_EXIT                 = 0X800,

    T2G_FSM_ERROR_HANDLE         = 0X1000,
    T2G_FSM_CMD_UNKNOWN          = 0X2000,
} T2G_cmd;



typedef enum T2G_error{
    T2G_ERROR_NONE,
    T2G_ERROR_FRAME1_CRC,
    T2G_ERROR_FRAME1_CTRL,
    T2G_ERROR_FRAME1_DATA,
    T2G_ERROR_DATA_CRC,
    T2G_ERROR_DATA_CTRL,
    T2G_ERROR_DATA,
    T2G_ERROR_TX_TIMEOUT,
    T2G_ERROR_RX_FRAME1_TIMEOUT,
    T2G_ERROR_RX_TIMEOUT,
    T2G_ERROR_TX,
    T2G_ERROR_CAL,
    T2G_ERROR_OPEN_TIMER,
    T2G_ERROR_UNKNOWN,
} T2G_error;

typedef struct T2G_status {
    T2G_eventHandle event;         //记录当前事件
    T2G_eventHandle prev_event;    //记录上一次事件

    T2G_cmd prev_cmd;        //记录上一次CMD
    T2G_cmd cmd;             //记录当前CMD
    T2G_cmd next_cmd;        //记录下一次CMD

    T2G_error error;         //记录T2G错误标识
    uint8_t ack;              //T2G ACK值
} T2G_status;


typedef struct st_rf_para{
    uint16_t default_rate;       //握手参数
    uint8_t default_power;
    uint8_t default_ch;

    uint8_t esl_id[ID_LEN];
    uint8_t ap_ch;                 //更新价签信道
    uint8_t ap_id;              //交互参数
    uint8_t recv_info_ch;       //子板接收上行信息信道
    uint16_t recv_info_rate;
    uint8_t recv_info_power;
    uint16_t FIFO_len;          //接收的FIFO长度
    uint16_t timeout;           //整体超时时间，单位ms
    uint32_t rf_timeout;        //实际写入射频配置的超时，单位us
    uint16_t data_len;          //数据长度。
    uint32_t frame1_timeout;    //帧1超时时间。
    uint8_t req_type;
}st_rf_para;

#pragma pack()

extern st_rf_para rf_para;

#endif /* ESL_TAP2GO_H_ */
