/*
 * rf_handle.c
 *
 *  Created on: 2020Äê10ÔÂ15ÈÕ
 *      Author: gaolongfei
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "rf_handle.h"
#include "thread_rf.h"
#include "task_id.h"
#include "update_type.h"
#include "debug.h"


typedef  int8_t (*cmd_start_func)(uint8_t** addr, uint8_t n, rf_parse_st* info);

extern int8_t group_wk_handle(uint8_t** addr, uint8_t n, rf_parse_st* info);
extern int8_t frame1_handle(uint8_t** addr, uint8_t n, rf_parse_st* info);
extern int8_t sleep_handle(uint8_t** addr, uint8_t n, rf_parse_st* info);
extern int8_t updata_handle(uint8_t** addr, uint8_t n, rf_parse_st* info);
extern int8_t query_handle(uint8_t** addr, uint8_t n, rf_parse_st* info);
extern int8_t set_wk_handle(uint8_t** addr, uint8_t n, rf_parse_st* info);

cmd_start_func cmd_start[HANDLE_MAX_NUM] = {set_wk_handle, group_wk_handle, frame1_handle, \
                                            sleep_handle, updata_handle, query_handle};


uint8_t* rf_cmd_head[HANDLE_MAX_NUM];        //todo: malloc rf_cmd_head


static void update_func(uint8_t* buf, uint8_t len);
static void debug_local_cmd(uint8_t **tmp, rf_parse_st* info);
static int8_t parse_cmd_data(uint8_t* addr, uint32_t left_len);
static void hb_func(uint8_t* buf, uint8_t len);
static void scan_bg_func(uint8_t* buf, uint8_t len);

volatile uint8_t core_idel_flag = 0;


rf_parse_st data_info={
.status = PARSE_START,
.cmd_left_len = 0
};



void rf_handle(rf_tsk_msg_t* msg)
{

    switch(msg->id){
        case CORE_CMD_ESL_UPDATA_REQUEST:
            update_func(msg->buf, msg->len);
            break;
        case CORE_CMD_ESL_HB_REQUEST:
            hb_func(msg->buf, msg->len);
            break;
        case CORE_CMD_SCAN_BG:
            scan_bg_func(msg->buf, msg->len);
        default:
            break;
    }
}

static void update_func(uint8_t* buf, uint8_t len)
{
    if (parse_cmd_data(buf, len) < 0){
        //todo: error
    }

    for (uint8_t i=0; i<HANDLE_MAX_NUM; i++){
        if (NULL != rf_cmd_head[i]){
            cmd_start[i](rf_cmd_head, i, &data_info);
            rf_cmd_head[i] = NULL;
        }
    }
}

static void hb_func(uint8_t* buf, uint8_t len)
{

}

static void scan_bg_func(uint8_t* buf, uint8_t len)
{

}


static int8_t parse_cmd_data(uint8_t* addr, uint32_t left_len)
{
    int8_t ret = 0;
    uint8_t* head_addr = addr;
    memset(rf_cmd_head, 0, sizeof(rf_cmd_head[HANDLE_MAX_NUM]));

    while(left_len > 0)
    {
        if (0 == data_info.cmd_left_len){
            data_info.rf_cmd = ((data_head_st*)head_addr)->cmd;
            data_info.cmd_total_len = ((data_head_st*)head_addr)->len + sizeof(data_head_st);
        }

        switch(data_info.rf_cmd)
        {
            case CMD_SET_WKUP_TRN:
            case CMD_SET_WKUP_GLB:
            case CMD_SET_WKUP_CH:
            case CMD_SET_WKUP_BDC:
            case CMD_SET_LED_FLASH:
                rf_cmd_head[SET_WK] = head_addr;
                break;
            case CMD_GROUPN_WKUP:
                rf_cmd_head[GROUP_WK] = head_addr;
                break;
            case CMD_GROUP1_FRAME1:
            case CMD_GROUP1_FRAME2:
            case CMD_GROUPN_FRAME1:
                rf_cmd_head[FRAME1] = head_addr;
                break;
            case CMD_GROUP1_SLEEP:
            case CMD_GROUPN_SLEEP:
                rf_cmd_head[SLEEP] = head_addr;
                break;
            case CMD_GROUP1_DATA:
            case CMD_GROUPN_DATA:
            case CMD_GROUPN_DATA_G2:
            case CMD_GROUPN_DATA_G1:
            case CMD_GROUP1_DATA_BDC:
            case CMD_GROUP1_DATA_NEWACK:
            case CMD_GROUPN_DATA_NEWACK:
                rf_cmd_head[UPDATA] = head_addr;
                break;
            case CMD_QUERY:
                rf_cmd_head[QUERY] = head_addr;
                break;
            case CMD_SET_WKUP:
            case CMD_GROUP1:
            case CMD_GROUPN:
                data_info.cmd_total_len = sizeof(data_head_st);
                break;
            default:
                left_len = 0;
                ret = -1;
                break;
        }

        if (data_info.cmd_total_len < left_len){
            head_addr += data_info.cmd_total_len;
            left_len -= data_info.cmd_total_len;
            data_info.cmd_left_len = 0;
            data_info.status = PARSE_END;
        } else {
            head_addr = NULL;
            left_len = 0;
            data_info.cmd_left_len = data_info.cmd_total_len - sizeof(data_head_st) - left_len;
            data_info.status = PARSE_DOING;
        }
    }

    debug_local_cmd(rf_cmd_head, &data_info);

    return ret;
}

static void debug_local_cmd(uint8_t **tmp, rf_parse_st* info)
{
    data_head_st *head = NULL;
    pdebug("++++++++++++++\n");
    head = (data_head_st*)tmp[SET_WK];
    pdebug("set cmd=0x%04X, addr=0x%08X, len = %d\r\n", head->cmd, head, head->len);
    head = (data_head_st*)tmp[GROUP_WK];
    pdebug("wkup addr=0x%08X, len=%d\n", head->cmd, head, head->len);
    head = (data_head_st*)tmp[FRAME1];
    pdebug("frame1 addr=0x%08X, len=%d\n", head->cmd, head, head->len);
    head = (data_head_st*)tmp[SLEEP];
    pdebug("sleep addr=0x%08X, len=%d\r\n", head->cmd, head, head->len);
    head = (data_head_st*)tmp[UPDATA];
    pdebug("updata cmd=0x%04X, addr=0x%08X,len=%d\n", head->cmd, head, head->len);
    head = (data_head_st*)tmp[QUERY];
    pdebug("query cmd=0x%04X, addr=0x%08X,len=%d\n", head->cmd, head, head->len);

    pdebug("last cmd=%d, total_len=%d, left_len=%d\n", info->rf_cmd, info->cmd_total_len);
    pdebug("status=%d, left_len=%d\n", info->status, info->cmd_left_len);
    pdebug("++++++++++++++\n");
}



uint8_t Core_GetQuitStatus(void)
{
    return core_idel_flag;
}
