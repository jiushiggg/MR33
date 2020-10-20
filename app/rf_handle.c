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


htpv3_cmd_addr rf_cmd_head;        //todo: malloc rf_data

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




int8_t parse_cmd_data(uint8_t* cmd_data_addr, uint32_t len)
{
    int8_t ret = 0;
    uint8_t* addr = cmd_data_addr;
    int32_t left_len = len;
    uint16_t cmd = 0;
    uint32_t cmd_len = 0;




    while(left_len > 0)
    {
        switch(data_info.status){
            case PARSE_START:
                cmd = ((data_head_st*)addr)->cmd;
                cmd_len = ((data_head_st*)addr)->len;

                data_info.rf_cmd = cmd;
                data_info.cmd_total_len = data_info.cmd_left_len = cmd_len;

                break;
            case PARSE_DOING:
                cmd = data_info.rf_cmd;
                cmd_len = data_info.cmd_total_len;
                break;
            case PARSE_END:
                break;
            default:
                break;
        }
        cmd = ((data_head_st*)addr)->cmd;
        cmd_len = ((data_head_st*)addr)->len
        pinfo("cmd=0x%04X, len=%d, addr=0x%08X, left len=%d\n",
                cmd, cmd_len, addr, left_len);
        switch(cmd)
        {
            case CMD_SET_WKUP_TRN:
            case CMD_SET_WKUP_GLB:
            case CMD_SET_WKUP_CH:
            case CMD_SET_WKUP_BDC:
            case CMD_SET_LED_FLASH:
                rf_data.set_addr = addr;
                break;
            case CMD_GROUPN_WKUP:
                rf_data.group_wk_addr = addr;
                break;
            case CMD_GROUP1_FRAME1:
            case CMD_GROUP1_FRAME2:
            case CMD_GROUPN_FRAME1:
                rf_data.frame1_addr = addr;
                break;
            case CMD_GROUP1_SLEEP:
            case CMD_GROUPN_SLEEP:
                rf_data.sleep_addr = addr
                break;
            case CMD_GROUP1_DATA:
            case CMD_GROUPN_DATA:
            case CMD_GROUPN_DATA_G2:
            case CMD_GROUPN_DATA_G1:
            case CMD_GROUP1_DATA_BDC:
            case CMD_GROUP1_DATA_NEWACK:
            case CMD_GROUPN_DATA_NEWACK:
                rf_data.updata_addr = addr;
                break;
            case CMD_SET_WKUP:
            case CMD_GROUP1:
            case CMD_GROUPN:
                addr += sizeof(data_head_st);
                left_len -= sizeof(data_head_st);
                cmd_len = 0;
                break;
            default:
                ret = -1;
                left_len = 0;
                break;
        }

        if (cmd_len > BUFFER_LEN){
            data_info.cmd_left_len -= cmd_len;
            addr += sizeof(data_head_st)+cmd_len;
            left_len -= sizeof(data_head_st)+cmd_len;
        }else {
            data_info.cmd_left_len -= left_len;
            addr = 0;
            left_len = 0;
            data_info.status = PARSE_DOING;
            return 0;
        }

    }



    debug_local_cmd();

    return ret;
}
