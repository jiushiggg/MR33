/*
 * rf_handle.c
 *
 *  Created on: 2020Äê10ÔÂ15ÈÕ
 *      Author: gaolongfei
 */

#include <stdint.h>
#include "rf_handle.h"
#include "thread_rf.h"
#include "task_id.h"
#include "update_type.h"


typedef struct _rf_cmd_type{
    uint16_t set_cmd;
    uint32_t set_len;
    uint32_t set_addr;

    uint16_t group_wk_cmd;
    uint32_t group_wk_len;
    uint32_t group_wk_addr;

    uint16_t frame1_cmd;
    uint32_t frame1_len;
    uint32_t frame1_addr;

    uint16_t updata_cmd;
    uint32_t updata_len;
    uint32_t updata_addr;

    uint16_t sleep_cmd;
    uint32_t sleep_len;
    uint32_t sleep_addr;
}rf_cmd_type;

typedef enum{
    PARSE_START,
    PARSE_DOING,
    PARSE_END
}em_parse_status;

typedef struct _rf_parse_st{
    em_parse_status status;
    uint16_t rf_cmd;
    uint32_t total_len;
    uint32_t left_len;
}rf_parse_st;


rf_cmd_type rf_cmd_head;        //todo: malloc rf_data

rf_parse_st data_info={
.status = PARSE_START
};






static void update_fnx(uint8_t buf, uint8_t len);

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

static void update_fnx(uint8_t buf, uint8_t len)
{

}




int32_t parse_cmd_data(uint8_t* cmd_data_addr, uint32_t cmd_data_len)
{
    int32_t ret = 0;
    uint8_t* addr = cmd_data_addr;
    int32_t left_data_len = cmd_data_len;
    uint16_t cmd = 0;
    uint32_t cmd_len = 0;


    while(left_data_len > 0)
    {
        cmd = ((data_cmd_st*)addr)->cmd;
        cmd_len = ((data_cmd_st*)addr)->len
        pinfo("cmd=0x%04X, len=%d, addr=0x%08X, left len=%d\n",
                cmd, cmd_len, addr, left_data_len);
        switch(cmd)
        {
            case CMD_SET_WKUP_TRN:
            case CMD_SET_WKUP_GLB:
            case CMD_SET_WKUP_CH:
            case CMD_SET_WKUP_BDC:
            case CMD_SET_LED_FLASH:
                rf_data.set_cmd = cmd;
                rf_data.set_addr = addr+sizeof(cmd)+sizeof(cmd_len);
                rf_data.set_len = cmd_len;
                addr += sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                left_data_len -= sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                ret += 1;
                break;
            case CMD_GROUPN_WKUP:
                rf_data.group_wk_cmd = cmd;
                rf_data.group_wk_addr = addr+sizeof(cmd)+sizeof(cmd_len);
                rf_data.group_wk_len = cmd_len;
                addr += sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                left_data_len -= sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                ret += 1;
                break;
            case CMD_GROUP1_FRAME1:
            case CMD_GROUP1_FRAME2:
            case CMD_GROUPN_FRAME1:
                rf_data.frame1_cmd = cmd;
                rf_data.frame1_addr = addr+sizeof(cmd)+sizeof(cmd_len);
                rf_data.frame1_len = cmd_len;
                addr += sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                left_data_len -= sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                ret += 1;
                break;
            case CMD_GROUP1_SLEEP:
            case CMD_GROUPN_SLEEP:
                rf_data.sleep_addr = addr+sizeof(cmd)+sizeof(cmd_len);
                rf_data.sleep_len = cmd_len;
                addr += sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                left_data_len -= sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                ret += 1;
                break;
            case CMD_GROUP1_DATA:
            case CMD_GROUPN_DATA:
            case CMD_GROUPN_DATA_G2:
            case CMD_GROUPN_DATA_G1:
            case CMD_GROUP1_DATA_BDC:
            case CMD_GROUP1_DATA_NEWACK:
            case CMD_GROUPN_DATA_NEWACK:
                rf_data.updata_cmd = cmd;
                rf_data.updata_addr = addr+sizeof(cmd)+sizeof(cmd_len);
                rf_data.updata_len = cmd_len;
                addr += sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                left_data_len -= sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                ret += 1;
                break;
            case CMD_SET_WKUP:
            case CMD_GROUP1:
            case CMD_GROUPN:
                addr += sizeof(cmd)+sizeof(cmd_len);
                left_data_len -= sizeof(cmd)+sizeof(cmd_len);
                break;
            default:
                ret = -1;
                left_data_len = 0;
                break;
        }

        switch(data_info.status){
            case PARSE_START:
                data_info.total_len = cmd_data_len
                break;
            case PARSE_DOING:
                break;
            case PARSE_END:
                break;
            default:
                break;
        }

    }



    debug_local_cmd();

    return ret;
}
