/*
 * update_type.h
 *
 *  Created on: 2020Äê10ÔÂ15ÈÕ
 *      Author: gaolongfei
 */

#ifndef ESL_MR33_UPDATE_TYPE_H_
#define ESL_MR33_UPDATE_TYPE_H_
#include <stdint.h>

#define ESL_ID_LEN  4
#define ESL_DATA_LEN    26


typedef struct _ack_data{
    uint8_t ctrl;
    uint8_t slot;   //
    uint8_t reserved[22];
    uint16_t crc;
}ack_data;

typedef struct _query_data{
    uint8_t ctrl;
    uint8_t slot;       //
    uint16_t pkg_num;   //total packet number
    uint8_t data[6];    //The first six bytes of the first packet
    uint8_t reserved[14];  //
    uint16_t crc;
}query_data;

typedef struct _basic_data_st{
    uint8_t id[ESL_ID_LEN];
    uint8_t channel;
    uint8_t len;
    uint8_t data[ESL_DATA_LEN];
}basic_data_st;

typedef struct _basic_data1_st{
    uint8_t id[ESL_ID_LEN];
    uint8_t channel;
    uint8_t set_sn;
    uint8_t len;
    uint8_t data[0];
}basic_data1_st;


typedef struct _set_wkup_st{
    uint16_t cmd;
    uint32_t len;
    uint16_t rate;
    uint8_t power;
    uint8_t duration;
    uint8_t slot_duration;
    uint8_t loop_time;
    uint8_t mode;
    uint8_t reserved[3];
    uint8_t main_channel;
    uint8_t n;
    uint8_t set_numbers;
    uint8_t set_wkup_data[0];
}set_wkup_st;

typedef struct _wkup_st{
    uint16_t cmd;
    uint32_t len;
    uint16_t rate;
    uint8_t power;
    uint8_t duration;
    uint8_t slot_duration;
    uint8_t reserved[8];
    uint8_t set_numbers;
    uint8_t data[0];
}wkup_st;

typedef struct  _data_head_st{
    uint16_t cmd;
    uint32_t len;
}data_head_st;

typedef struct _mixed_group_f1_st{
    uint16_t cmd;
    uint32_t len;
    uint16_t rate;
    uint8_t power;
    uint8_t duration;
    uint8_t mode;
    uint8_t solt_duration;
    uint8_t reserved[7];
    uint16_t num;
    basic_data_st data[0];
}mixed_group_f1_st;


typedef struct _group_data_st{
    uint16_t cmd;
    uint32_t len;
    uint8_t recv_id[4];
    uint8_t power;
    uint16_t tx_rate;
    uint16_t rx_rate;
    uint8_t esl_worktime;
    uint8_t ctrl_bit;
    uint8_t mode;
    uint8_t deal_duration;
    uint8_t tx_interval;
    uint16_t tx_duration;
    uint8_t channel;
    uint8_t reserved[4];
    uint16_t num;
    uint8_t data[0];
}group_data_st;

typedef struct _sleep_st{
    uint16_t cmd;
    uint32_t len;
    uint16_t rate;
    uint8_t power;
    uint8_t mode;
    uint8_t interval;
    uint8_t ctrl_bit;
    uint8_t times;
    uint8_t default_len;
    uint8_t reserved[7];
    uint16_t num;
    uint8_t data[0];
}sleep_st;



typedef struct _frame1_st{
    uint16_t cmd;
    uint32_t len;
    uint16_t rate;
    uint8_t power;
    uint8_t duration;
    uint8_t mode;
    uint8_t reserved[8];
    uint16_t num;
    basic_data_st data[0];
}frame1_st;

typedef struct _query_st{
    uint16_t cmd;
    uint32_t len;
    uint8_t recv_id_id[4];
    uint8_t power;
    uint16_t tx_rate;
    uint16_t rx_rate;
    uint8_t esl_worktime;
    uint8_t ctrl_bit;
    uint8_t mode;
    uint8_t deal_duration;
    uint8_t tx_interval;
    uint16_t tx_duration;
    uint8_t channel;
    uint8_t reserved[4];
    uint16_t num;
    uint8_t data[0][26];
}query_st;


#define CMD_SET_WKUP            0x0100
#define CMD_SET_WKUP_TRN        0x0110
#define CMD_SET_WKUP_GLB        0x0120
#define CMD_SET_WKUP_CH         0x0130
#define CMD_SET_WKUP_BDC        0x0101
#define CMD_SET_LED_FLASH       0x0103

#define CMD_GROUP1              0x0200
#define CMD_GROUP1_FRAME1       0x0240
#define CMD_GROUP1_FRAME2       0x0241
#define CMD_GROUP1_DATA         0x0250
#define CMD_GROUP1_DATA_NEWACK  0x0251
#define CMD_GROUP1_DATA_BDC     0x0280
#define CMD_GROUP1_SLEEP        0x02B0

#define CMD_GROUPN              0x0300
#define CMD_GROUPN_WKUP         0x0310
#define CMD_GROUPN_FRAME1       0x0340
#define CMD_GROUPN_DATA         0x0350
#define CMD_GROUPN_DATA_NEWACK  0x0351
#define CMD_GROUPN_DATA_G1      0x0370
#define CMD_GROUPN_DATA_G2      0x0360
#define CMD_GROUPN_SLEEP        0x03B0

#define CMD_QUERY               0x03A0

#define CMD_ACK                 0x0900
#define CMD_ACK_NEW             0x0901


#endif /* ESL_MR33_UPDATE_TYPE_H_ */
