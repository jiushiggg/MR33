/*
 * wakeup.c
 *
 *  Created on: 2020Äê10ÔÂ21ÈÕ
 *      Author: gaolongfei
 */

#include "update_type.h"
#include "rf_handle.h"
#include "debug.h"

#include "cc2640r2_rf.h"
#include "timer.h"


static int8_t set_wakeup_led_flash(void* addr, void* f1_addr);
static int8_t wakeup_start(void* addr, uint8_t type);


int8_t set_wk_handle(uint8_t** addr, uint8_t n, rf_parse_st* info)
{
    int8_t ret = 0;
    set_wkup_st* set = (set_wkup_st*)addr[n];
    frame1_st* frame1 = (frame1_st*)addr[FRAME1];

    TRACE();

    if((set->cmd==CMD_SET_WKUP_TRN)||(set->cmd==CMD_SET_WKUP_BDC))
    {
        pdebug("set wkup trn & bdc, loop:%d\n", set->loop_time);
        wakeup_start(set, 0);
    }
    else if((set->cmd==CMD_SET_WKUP_GLB) || (set->cmd==CMD_SET_WKUP_CH))
    {
        pinfo("sw1 bg\n");
        wakeup_start(set, 1);

    }
    else if (set->cmd == CMD_SET_LED_FLASH)
    {
        pinfo("sw2 bg\n");
        set_wakeup_led_flash(set, frame1);
        frame1 = NULL;
    }

    return ret;
}

int8_t group_wk_handle(uint8_t** addr, uint8_t n, rf_parse_st* info)
{
    wkup_st *group = (wkup_st*)addr[n];
    pinfo("sw1 bg\r\n");
    wakeup_start(group, 1);
    return 0;
}



#define RF_CHANING_MODE

static int8_t wakeup_start(void* addr, uint8_t type)
{
    wkup_st* wkup_addr =  (wkup_st*)addr;
    basic_data_st *basic_data = (basic_data_st *)wkup_addr->data;
    int8_t ret = 0;
    uint8_t timer = 0;
    uint16_t timer_count = 0;
    uint32_t duration_ms = 0;
    uint8_t ctrl = 0;

#ifdef RF_CHANING_MODE
    uint8_t *data = NULL;
#else
    uint8_t data[SIZE_ESL_DATA_BUF] = {0};
#endif

    RF_EventMask result;
    uint8_t  pend_flg = PEND_STOP;
#ifdef RF_CHANING_MODE
    send_chaningmode_init();
    write2buf = listInit();
#endif
    pdebug("wkup addr=0x%08X, len=%d\r\n", addr, wkup_addr->len);

    if((addr==NULL))
    {
        ret = -1;
        goto done;
    }
    if (0 == wkup_addr->len){
        ret = -1;
        goto done;
    }

    pdebug("wkup para: datarate=%d, power=%d, duration=%d, slot_duration=%d\n",
           wkup_addr->rate, wkup_addr->power, wkup_addr->duration, wkup_addr->slot_duration);

    if(wkup_addr->duration == 0)
    {
        pdebug("warning: wkup duration is 0\r\n");
        goto done;
    }
#ifdef RF_CHANING_MODE
    data = ((MyStruct*)write2buf)->tx->pPkt;
#endif
    memcpy(data, basic_data->data, basic_data->len);

    pdebug("wkup id:0x%02X-0x%02X-0x%02X-0x%02X, channel=%d, len=%d, data=", \
           basic_data->id[0], basic_data->id[1], basic_data->id[2], basic_data->id[3], basic_data->channel, basic_data->len);
    pdebughex(data, basic_data->len);

    ctrl = data[0];


//#define GGG_DEBUG
#ifdef GGG_DEBUG
    slot_duration = 10;
    duration = 4;
    interval = 23;
    datarate = DATA_RATE_500K;
    id[0] = 52; id[1] = 0x56; id[2] = 0x78; id[3] = 0x53;
    channel = 2;
    data_len = 26;
#endif


    set_power_rate(wkup_addr->power, wkup_addr->rate);
    set_frequence(basic_data->channel);
#ifdef RF_CHANING_MODE
#else
    send_data_init(basic_data->id, basic_data->data, basic_data->data_len, 5000);
#endif

    if(1 == ((set_wkup_st*)addr)->mode)
    {
        duration_ms = 200;
    }
    else
    {
        duration_ms = (uint32_t)wkup_addr->duration * 1000 - 500;
    }

    if((timer=TIM_Open(wkup_addr->slot_duration, duration_ms/wkup_addr->slot_duration, TIMER_DOWN_CNT, TIMER_PERIOD)) == TIMER_UNKNOW)
    {
        perr("g3_wkup() open timer.\r\n");
        ret = -4;
        goto done;
    }

    while(TIME_COUNTING==TIM_CheckTimeout(timer))
    {
        if(Core_GetQuitStatus() == 1)
        {
            pdebug("g3_wkup quit\r\n");
            break;
        }
#ifdef RF_CHANING_MODE
        data = ((MyStruct*)write2buf)->tx->pPkt;
#endif
        if(type == 0) // 0 is default
        {
            timer_count = TIM_GetCount(timer);

            if(ctrl == 0xAA)
            {
                data[0] = ctrl;
            }
            else
            {
                data[0] = (ctrl&0xE0) | ((timer_count >> 8) & 0x1f);
            }

            data[1] = timer_count & 0xff;
        }


#ifdef RF_CHANING_MODE
        if (PEND_STOP == pend_flg){
            pend_flg = PEND_START;
            result = send_chaningmode(basic_data->id, data, basic_data->len, 6000);
            memcpy(((MyStruct*)write2buf->next)->tx->pPkt, data, basic_data->len);
            write2buf = List_next(write2buf);
        }else{
            RF_wait_cmd_finish();
            //write2buf = List_next(write2buf);
        }
#else
        if (PEND_START == pend_flg){
            send_pend(result);
        }
        result = send_async(rf_time);
        pend_flg = PEND_START;
#endif
    }
#ifdef RF_CHANING_MODE
    RF_wait_cmd_finish(); //Wait for the last packet to be sent
    RF_cancle(result);
#endif
    TIM_Close(timer);

    ret = 1;
done:
    return ret;
}


static int8_t set_wakeup_led_flash(void* addr, void* f1_addr)
{
    set_wkup_st* wkup_addr =  (set_wkup_st*)addr;
    basic_data_st *basic_data = (basic_data_st *)(wkup_addr->data);
    frame1_st* frame1_addr = (frame1_st*)f1_addr;
    basic_data_st *f1_basic_data = frame1_addr->data;
    int8_t ret = 0;
    uint8_t timer = 0;
    uint32_t duration_ms = 0;
    uint8_t j = 0;

    pdebug("wkup addr=0x%08X, f1 addr=0x%08X, len=%d\r\n", wkup_addr, frame1_addr, wkup_addr->len);
    if(wkup_addr==NULL || frame1_addr==NULL)
    {
        ret = -1;
        goto done;
    }

    if (0 == wkup_addr->len){
        ret = -1;
        goto done;
    }


    pdebug("wkup para: datarate=%d, power=%d, duration=%d, slot_duration=%d\r\n", \
           wkup_addr->rate, wkup_addr->power, wkup_addr->duration, wkup_addr->slot_duration);

    if(wkup_addr->duration == 0)
    {
        pdebug("warning: wkup duration is 0\r\n");
        goto done;
    }


    pinfo("wkup data: id:0x%02X-0x%02X-0x%02X-0x%02X, channel=%d, len=%d, data=", \
          basic_data->id[0], basic_data->id[1], basic_data->id[2], basic_data->id[3], basic_data->channel, basic_data->len);

#if FLASH_LED_TEST
    pinfo("num:%d ", group_data->group_num);
    for (j=0; j< group_data->group_num; j++){
        pdebughex1((uint8_t*)&group_data->data[j], 32);
    }
    j = 0;
#endif
//#define GGG_DEBUG
#ifdef GGG_DEBUG
    slot_duration = 10;
    duration = 4;
    interval = 23;
    datarate = DATA_RATE_500K;
    id[0] = 52; id[1] = 0x56; id[2] = 0x78; id[3] = 0x53;
    channel = 2;
    data_len = 26;
#endif


    set_power_rate(wkup_addr->power, wkup_addr->rate);
    set_frequence(basic_data->channel);
    if(wkup_addr->mode == 1)
    {
        duration_ms = (uint32_t)wkup_addr->duration * 10;
    }
    else
    {
        duration_ms = (uint32_t)wkup_addr->duration * 1000 - 700;
    }

    if((timer=TIM_Open(wkup_addr->slot_duration, duration_ms/wkup_addr->slot_duration, TIMER_DOWN_CNT, TIMER_PERIOD)) == TIMER_UNKNOW)
    {
        perr("g3_wkup() open timer.\r\n");
        ret = -4;
        goto done;
    }
#ifdef FLASH_LED_TEST
    data[0] = 0xE0;
    data[1] = 0;
    data[2] = 0xff;
    data[3] = 0xff;
    data[4] = 0xff;
    data[5] = 0x0;
#endif
    while(TIME_COUNTING==TIM_CheckTimeout(timer))
    {
        if(Core_GetQuitStatus() == 1)
        {
            pdebug("g3_wkup quit\r\n");
            break;
        }
        basic_data->data[0] = (basic_data->data[0]&0xE0) | (j+1);
        send_flash_led_data(basic_data->id, basic_data->data, f1_basic_data[j].id, f1_basic_data[j].data);
        j = ++j >= frame1_addr->num ? 0 : j;
    }

    TIM_Close(timer);
    ret = 1;
done:
    return ret;
}
