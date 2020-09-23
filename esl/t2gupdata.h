/*
 * t2gupdata.h
 *
 *  Created on: 2018Äê12ÔÂ4ÈÕ
 *      Author: ggg
 */
#ifndef T2GUPDATA_H_
#define T2GUPDATA_H_
#include "datatype.h"
#include "updata.h"


#define T2G_MAX_FAILED_PKG_NUM 9
#define T2G_AP_QUERY_CTRL      ((0x05<<3)|0x01)
#define T2G_AP_RESPONSE_CTRL   ((0x05<<3)|0x01)
#define T2G_SLEEP_CTRL         ((0x1F<<3)|0x01)

#pragma pack(1)

typedef struct st_t2g_ap_query{
    uint8_t ctrl;
    uint8_t sid;
    uint8_t slot;
    uint16_t total_packet;
    uint8_t reserve[19];
    uint16_t crc;
}st_t2g_ap_query;

typedef struct st_t2g_esl_rsp{
    uint8_t ctrl;
    uint8_t sid;
    uint8_t slot;
    uint16_t lost_num;
    uint8_t reserve;
    uint8_t lost_packet[T2G_MAX_FAILED_PKG_NUM*2];
    uint16_t crc;
}st_t2g_esl_rsp;

typedef struct st_t2g_esl_sleep{
    uint8_t ctrl;
    uint16_t total_pkg;
    uint8_t reserve[21];
    uint16_t crc;
}st_t2g_esl_sleep;


#pragma pack()

extern UINT16 t2g_init_data(UINT32 addr, UINT32 len, updata_table_t *table);
extern UINT8 t2g_updata_loop(updata_table_t *table);
extern void t2g_make_ack(updata_table_t *table, UINT32 *ack_addr, UINT32 *ack_len, UINT8 sn_offset);
extern void t2g_make_new_ack(updata_table_t *table, UINT32 *ack_addr, UINT32 *ack_len, UINT8 sn_offset);
#endif



