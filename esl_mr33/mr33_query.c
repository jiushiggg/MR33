/*
 * mr33_query.c
 *
 *  Created on: 2020Äê10ÔÂ21ÈÕ
 *      Author: gaolongfei
 */


#include "update_type.h"
#include "rf_handle.h"
#include "debug.h"

#include "cc2640r2_rf.h"
#include "timer.h"


int8_t query_handle(uint8_t* addr[], uint8_t n, rf_parse_st* info, void * extra)
{
    int32_t ret = 0;
    query_st* query =  (query_st*)addr[n];
    uint32_t deal_timeout = query->deal_duration*1000;
    basic_data_st *tmp = query->esl;

    for(uint16_t i = 0; i < query->num; i++)
    {
        if(Core_GetQuitStatus() == 1)
        {
            pdebug("m1_query_miss quit\r\n");
            break;
        }

//        if(TIM_CheckTimeout(timer) == TIME_OUT)
//        {
//            pdebug("mode0_query_miss_round, timer timeout.\r\n");
//            break;
//        }

        set_power_rate(query->power, query->tx_rate);
        set_frequence(tmp[i].channel);
        send_data(tmp[i].id, tmp[i].data, tmp[i].len, 2000);
        set_power_rate(RF_DEFAULT_POWER, query->rx_rate);
        set_frequence(tmp[i].channel);
        if(recv_data(query->recv_id, tmp[i].data, tmp[i].len, deal_timeout) == 0)
        {
            pinfo("rec timeout:%d\r\n", deal_timeout);
            memset(tmp[i].data, 0, tmp[i].len);
        }

    }
    return ret;
}
