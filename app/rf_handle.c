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




int32_t parse_cmd_data(uint32_t cmd_data_addr, uint32_t cmd_data_len)
{
    int32_t ret = 0;
    uint32_t addr = cmd_data_addr;
    int32_t left_data_len = cmd_data_len;
    uint16_t cmd = 0;
    uint32_t cmd_len = 0;


    while(left_data_len > 0)
    {
        cmd = g3_get_cmd(addr, &cmd_len);
        pdebug("get cmd=0x%04X, cmd_len=%d, cmd_addr=0x%08X, left len=%d.\r\n",
                cmd, cmd_len, addr, left_data_len);

        switch(cmd)
        {
            case CMD_SET_WKUP_TRN:
            case CMD_SET_WKUP_GLB:
            case CMD_SET_WKUP_CH:
            case CMD_SET_WKUP_BDC:
            case CMD_SET_LED_FLASH:
                set_cmd = cmd;
                set_addr = addr+sizeof(cmd)+sizeof(cmd_len);
                set_len = cmd_len;
                addr += sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                left_data_len -= sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                ret += 1;
                if(cmd == CMD_SET_WKUP_BDC)
                {
                    set_loop_times = wakeup_get_loop_times(set_addr);
                }
                break;
            case CMD_GROUPN_WKUP:
                wkup_addr = addr+sizeof(cmd)+sizeof(cmd_len);
                wkup_len = cmd_len;
                addr += sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                left_data_len -= sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                ret += 1;
                break;
            case CMD_GROUP1_FRAME1:
            case CMD_GROUP1_FRAME2:
            case CMD_GROUPN_FRAME1:
                frame1_cmd = cmd;
                frame1_addr = addr+sizeof(cmd)+sizeof(cmd_len);
                frame1_len = cmd_len;
                addr += sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                left_data_len -= sizeof(cmd)+sizeof(cmd_len)+cmd_len;
                ret += 1;
                break;
            case CMD_GROUP1_SLEEP:
            case CMD_GROUPN_SLEEP:
                sleep_addr = addr+sizeof(cmd)+sizeof(cmd_len);
                sleep_len = cmd_len;
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
                updata_cmd = cmd;
                updata_addr = addr+sizeof(cmd)+sizeof(cmd_len);
                updata_len = cmd_len;
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

    }

    debug_local_cmd();

    return ret;
}
