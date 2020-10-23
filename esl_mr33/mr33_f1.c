/*
 * mr33_f1.c
 *
 *  Created on: 2020Äê10ÔÂ21ÈÕ
 *      Author: gaolongfei
 */

#include "update_type.h"
#include "rf_handle.h"
#include "debug.h"

#include "cc2640r2_rf.h"
#include "timer.h"
#include "crc16.h"

static uint8_t frame2(void* addr, uint8_t num, int32_t duration);
static uint8_t frame1_mode0(void* addr, uint8_t num, int32_t duration);


int8_t frame1_handle(uint8_t** addr, uint8_t n, rf_parse_st* info, void * extra)
{
    int8_t ret = 0;
    int32_t dur = 0;
    frame1_st* f1 =  (frame1_st*)addr[n];
    basic_data_st *basic_data = (basic_data_st *)f1->data;

    if((addr==NULL))
    {
        ret = -1;
        goto done;
    }
    if (0 == f1->len){
        ret = -1;
        goto done;
    }

    if(f1->cmd == CMD_GROUP1_FRAME2)
    {
        dur = f1->duration * 1000;//in frame2 unit is s
    }
    else
    {
        dur = f1->duration;
    }

    pdebug("frame1:datarate=%d,power=%d,duration=%dms,mode=%d,num=%d\r\n",
           f1->rate, f1->power, dur,
           f1->mode, f1->num);

    if(f1->duration == 0)
    {
        pdebug("warning: frame 1 duration is 0\r\n");
        goto done;
    }

    set_power_rate(f1->power, f1->rate);

    if(f1->cmd == CMD_GROUP1_FRAME2)
    {
        ret = frame2(basic_data, f1->num, dur);
    }
    else
    {
        switch(f1->mode)
        {
            case 0: // 0 is default
            default:
                if(frame1_mode0(basic_data, f1->num, dur) == 1)
                {
                    ret = 1;
                }
            break;
        }
    }

done:
    pdebug("frame1 end\r\n");
    return ret;
}




static uint8_t frame2(void* addr, uint8_t num, int32_t duration)
{
    uint8_t ret = 0;
    int16_t i = 0;
    uint8_t timer = 0;
    basic_data_st* cur = (basic_data_st*)addr;
    uint16_t timercount=0, crc = 0;
    RF_EventMask result;
    uint8_t pend_flg = PEND_STOP;

    if((timer=TIM_Open(10, duration/10, TIMER_DOWN_CNT, TIMER_PERIOD)) == TIMER_UNKNOW) //TODO: mode right?
    {
        perr("frame2_mode0() open timer.\r\n");
        goto done;
    }

    i = 0;
    while(1)
    {
        if(Core_GetQuitStatus() == 1)
        {
            pdebug("_frame2 quit1\r\n");
            break;
        }

        if(TIM_CheckTimeout(timer) == TIME_OUT)
        {
            pdebug("_frame2() timeout\r\n");
            ret = 1;
            break;
        }

        timercount = TIM_GetCount(timer);
        memcpy(cur[i].data+2, &timercount, sizeof(timercount));
        crc = 0;
        crc = CRC16_CaculateStepByStep(crc, cur[i].data, 24);
        crc = CRC16_CaculateStepByStep(crc, cur[i].id, 4);
        memcpy(cur[i].data+24, &crc, sizeof(crc));

        /*
        pdebug("_frame2 %d: id=0x%02X-0x%02X-0x%02X-0x%02X, channel=%d, len=%d, data=", \
                i, id[0], id[1], id[2], id[3], channel, len);
        pdebughex(data, len);
        */
        if (PEND_START == pend_flg){
            send_pend(result);
        }
        result = send_without_wait(cur[i].id, cur[i].data, cur[i].len, cur[i].channel, 6000);
        pend_flg = PEND_START;

        i = ++i >= num ? 0 : i;
    }

    TIM_Close(timer);
    wait(1000);

done:
    return ret;
}


static uint8_t frame1_mode0(void* addr, uint8_t num, int32_t duration)
{
    uint8_t ret = 0;
    uint16_t i = 0;
    uint8_t timer = 0;
    basic_data_st* cur = (basic_data_st*)addr;
    RF_EventMask result;
    uint8_t pend_flg = PEND_STOP;

    if((timer=TIM_Open(duration, 1, TIMER_UP_CNT, TIMER_ONCE)) == TIMER_UNKNOW)
    {
        perr("g3_send_frame1_mode0() open timer.\r\n");
        goto done;
    }

    i = 0;
    cur = addr;
    while(1)
    {
        if(Core_GetQuitStatus() == 1)
        {
            pdebug("frame1_mode0 quit1\r\n");
            break;
        }

        if(TIM_CheckTimeout(timer)==TIME_OUT)
        {
            pdebug("g3_send_frame1_mode0() timer timeout.\r\n");
            ret = 1;
            break;
        }

        /*
        pdebug("g3_send_frame1_mode0 frame1 %d: id=0x%02X-0x%02X-0x%02X-0x%02X, channel=%d, len=%d, data=", \
                i, id[0], id[1], id[2], id[3], channel, len);
        pdebughex(data, len);
        */
        if (PEND_START == pend_flg){
            send_pend(result);
        }
        result = send_without_wait(cur[i].id, cur[i].data, cur[i].len, cur[i].channel, 6000);
        pend_flg = PEND_START;

        i = ++i >= num ? 0 : i;
    }
    TIM_Close(timer);
    wait(1000);

done:
    return ret;
}
