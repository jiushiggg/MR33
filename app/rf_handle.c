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


typedef struct _htpv3_cmd_addr{
    uint8_t* set_addr;
    uint8_t* group_wk_addr;
    uint8_t* frame1_addr;
    uint8_t* updata_addr;
    uint8_t* sleep_addr;
}htpv3_cmd_addr;

typedef enum{
    PARSE_START,
    PARSE_DOING,
    PARSE_END
}em_parse_status;

typedef struct _rf_parse_st{
    em_parse_status status;
    uint16_t rf_cmd;
    uint32_t cmd_total_len;
    uint32_t cmd_left_len;
}rf_parse_st;


static void update_fnx(uint8_t* buf, uint8_t len);

htpv3_cmd_addr rf_cmd_head;        //todo: malloc rf_cmd_head

rf_parse_st data_info={
.status = PARSE_START,
.cmd_left_len = 0
};








void rf_handle(rf_tsk_msg_t* msg)
{

    switch(msg->id){
        case CORE_CMD_ESL_UPDATA_REQUEST:
            update_fnx(msg->buf, msg->len);
            break;
        default:
            break;
    }
}

static void update_fnx(uint8_t* buf, uint8_t len)
{

}

void debug_local_cmd(htpv3_cmd_addr* tmp, rf_parse_st* info)
{
    data_head_st *head = NULL;
    pdebug("++++++++++++++\n");
    head = (data_head_st*)tmp->set_addr;
    pdebug("set cmd=0x%04X, addr=0x%08X, len = %d\r\n", head->cmd, head, head->len);
    head = (data_head_st*)tmp->group_wk_addr;
    pdebug("wkup addr=0x%08X, len=%d\n", head->cmd, head, head->len);
    head = (data_head_st*)tmp->frame1_addr;
    pdebug("frame1 addr=0x%08X, len=%d\n", head->cmd, head, head->len);
    head = (data_head_st*)tmp->sleep_addr;
    pdebug("sleep addr=0x%08X, len=%d\r\n", head->cmd, head, head->len);
    head = (data_head_st*)tmp->updata_addr;
    pdebug("updata cmd=0x%04X, addr=0x%08X,len=%d\n", head->cmd, head, head->len);

    pdebug("last cmd=%d, total_len=%d, left_len=%d\n", info->rf_cmd, info->cmd_total_len);
    pdebug("status=%d, left_len=%d\n", info->status, info->cmd_left_len);
    pdebug("++++++++++++++\n");
}


int8_t parse_cmd_data(uint8_t* addr, uint32_t left_len)
{
    int8_t ret = 0;
    uint8_t* head_addr = addr;
    memset(&rf_cmd_head, 0, sizeof(htpv3_cmd_addr));

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
                rf_cmd_head.set_addr = head_addr;
                break;
            case CMD_GROUPN_WKUP:
                rf_cmd_head.group_wk_addr = head_addr;
                break;
            case CMD_GROUP1_FRAME1:
            case CMD_GROUP1_FRAME2:
            case CMD_GROUPN_FRAME1:
                rf_cmd_head.frame1_addr = head_addr;
                break;
            case CMD_GROUP1_SLEEP:
            case CMD_GROUPN_SLEEP:
                rf_cmd_head.sleep_addr = head_addr;
                break;
            case CMD_GROUP1_DATA:
            case CMD_GROUPN_DATA:
            case CMD_GROUPN_DATA_G2:
            case CMD_GROUPN_DATA_G1:
            case CMD_GROUP1_DATA_BDC:
            case CMD_GROUP1_DATA_NEWACK:
            case CMD_GROUPN_DATA_NEWACK:
                rf_cmd_head.updata_addr = head_addr;
                break;
            case CMD_SET_WKUP:
            case CMD_GROUP1:
            case CMD_GROUPN:
                data_info.cmd_total_len = sizeof(data_head_st);
                break;
            default:
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

    debug_local_cmd(&rf_cmd_head, &data_info);

    return ret;
}
