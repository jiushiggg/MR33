#include "cc2640r2_rf.h"
#include "wakeup.h"
#include "flash.h"
#include "timer.h"
#include "debug.h"
#include "data.h"
#include "bsp.h"
#include "core.h"
#include "crc16.h"

#define MAX_GROUP_NUM	24

typedef struct _st_basic_data{
	st_basic_data_head head;
	uint8_t data[SIZE_ESL_DATA_BUF];
}st_basic_data;

typedef struct st_led_flash_data{
	uint16_t group_num;
	st_basic_data data[MAX_GROUP_NUM];
}st_led_flash_data;

#define RF_CHANING_MODE
INT32 wakeup_start(UINT32 addr, UINT32 len, UINT8 type)
{
	INT32 ret = 0;
	UINT8 timer = 0;
	UINT32 duration_ms = 0;
	UINT16 timer_count = 0;
	
	UINT8 id[4] = {0};
#ifdef RF_CHANING_MODE
    UINT8 *data = NULL;
#else
    UINT8 data[SIZE_ESL_DATA_BUF] = {0};
    uint32_t rf_time = 0;
#endif
	UINT8 data_len = 0;
	UINT8 power = 0;
	UINT8 channel = 0;
	UINT16 datarate = 0;
	UINT8 duration = 0;
	UINT8 slot_duration = 0;
	UINT8 ctrl = 0;
	UINT16 interval = 0;
	UINT8 mode = 0;
	RF_EventMask result;
	uint8_t  pend_flg = PEND_STOP;

#ifdef RF_CHANING_MODE
	send_chaningmode_init();
    write2buf = listInit();
#endif
	pdebug("wkup addr=0x%08X, len=%d\r\n", addr, len);

	if((addr==0) || (len==0))
	{
		ret = -1;
		goto done;
	}

	if(g3_get_wkup_para(addr, &datarate, &power, &duration, &slot_duration, &mode) == 0)
	{
		perr("g3_wkup() get para from flash.\r\n");
		ret = -2;
		goto done;
	}

	interval = g3_get_wkup_interval(addr);
	if(interval == 0)
	{
		interval = 30;
	}

	pdebug("wkup para: datarate=%d, power=%d, duration=%d, slot_duration=%d, interval=%d\r\n", \
			datarate, power, duration, slot_duration, interval);

	if(duration == 0)
	{
		pdebug("warning: wkup duration is 0\r\n");
		goto done;
	}
#ifdef RF_CHANING_MODE
	data = ((MyStruct*)write2buf)->tx->pPkt;
#endif
	if(get_one_data(addr+OFFSET_WKUP_DATA, id, &channel, &data_len, data, SIZE_ESL_DATA_BUF) == 0)
	{
		perr("g3_wkup() get data from flash.\r\n");
		ret = -3;
		goto done;
	}

	pdebug("wkup data: id:0x%02X-0x%02X-0x%02X-0x%02X, channel=%d, len=%d, data=", \
			id[0], id[1], id[2], id[3], channel, data_len);
	pdebughex(data, data_len);

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


    set_power_rate(power, datarate);
    set_frequence(channel);
#ifdef RF_CHANING_MODE
#else
    send_data_init(id, data, data_len, 5000);
    rf_time = RF_getCurrentTime();
#endif
	if(mode == 1)
	{
		duration_ms = 200;
	}
	else
	{
		duration_ms = (uint32_t)duration * 1000 - 500;
	}
	
	if((timer=TIM_Open(slot_duration, duration_ms/slot_duration, TIMER_DOWN_CNT, TIMER_PERIOD)) == TIMER_UNKNOW)
	{
		perr("g3_wkup() open timer.\r\n");
		ret = -4;
		goto done;
	}
	LED_TOGGLE(DEBUG_IO0);
	interval = EasyLink_us_To_RadioTime(interval);
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
		LED_ON(DEBUG_IO1);
#ifdef RF_CHANING_MODE
        if (PEND_STOP == pend_flg){
            pend_flg = PEND_START;
            result = send_chaningmode(id, data, data_len, 6000);
            memcpy(((MyStruct*)write2buf->next)->tx->pPkt, data, data_len);
			write2buf = List_next(write2buf);
        }else{
            RF_wait_cmd_finish();
            //write2buf = List_next(write2buf);
        }
#else
        rf_time += interval;
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
	LED_TOGGLE(DEBUG_IO0);
	TIM_Close(timer);
	
	ret = 1;
done:
	return ret;
}

INT32 set_wakeup_led_flash(UINT32 set_addr, UINT32* f1_addr, void *buf, UINT32 len)
{
	INT32 ret = 0;
	UINT8 timer = 0;
	UINT32 duration_ms = 0;
	UINT8 j = 0;
	UINT8 id[4] = {0};
    UINT8 data[SIZE_ESL_DATA_BUF] = {0};
	UINT8 data_len = 0;
	UINT8 power = 0;
	UINT8 channel = 0;
	UINT16 datarate = 0;
	UINT8 duration = 0;
	UINT8 slot_duration = 0;
	UINT8 mode = 0;
	uint8_t offset = 0;
	st_led_flash_data *group_data = (st_led_flash_data*)buf;

	pdebug("wkup addr=0x%08X, f1 addr=0x%08X, len=%d\r\n", set_addr, *f1_addr, len);

	if((set_addr==0) || (len==0) || *f1_addr==0)
	{
		ret = -1;
		goto done;
	}

	if(g3_get_wkup_para(set_addr, &datarate, &power, &duration, &slot_duration, &mode) == 0)
	{
		perr("g3_wkup() get para from flash.\r\n");
		ret = -2;
		goto done;
	}

	pdebug("wkup para: datarate=%d, power=%d, duration=%d, slot_duration=%d\r\n", \
			datarate, power, duration, slot_duration);

	if(duration == 0)
	{
		pdebug("warning: wkup duration is 0\r\n");
		goto done;
	}

	offset=get_one_data(set_addr+OFFSET_WKUP_DATA, id, &channel, &data_len, data, SIZE_ESL_DATA_BUF);
	if(offset == 0)
	{
		perr("g3_wkup() get data from flash.\r\n");
		ret = -3;
		goto done;
	}

	pinfo("wkup data: id:0x%02X-0x%02X-0x%02X-0x%02X, channel=%d, len=%d, data=", \
			id[0], id[1], id[2], id[3], channel, data_len);

	if(0 == get_flash_led_data(*f1_addr+OFFSET_FRAME1_DATA, group_data, sizeof(st_led_flash_data)))
	{
		perr("g3_wkup() get data from flash.\r\n");
		ret = -3;
		goto done;
	}
	*f1_addr = 0;

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


    set_power_rate(power, datarate);
    set_frequence(channel);
	if(mode == 1)
	{
		duration_ms = (uint32_t)duration * 10;
	}
	else
	{
		duration_ms = (uint32_t)duration * 1000 - 500;
	}

	if((timer=TIM_Open(slot_duration, duration_ms/slot_duration, TIMER_DOWN_CNT, TIMER_PERIOD)) == TIMER_UNKNOW)
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
		data[0] = (data[0]&0xE0) | (j+1);
		send_flash_led_data(id, data, group_data->data[j].head.id, group_data->data[j].data);
		j = ++j >= group_data->group_num ? 0 : j;
	}

	TIM_Close(timer);
	ret = 1;
done:
	return ret;
}


#if 0
#undef RF_CHANING_MODE
INT32 multi_set_wakeup_start(UINT32 addr, UINT32 len)
{
    INT32 ret = 0;
    UINT8 timer = 0;
    UINT32 duration_ms = 0;
    UINT16 timer_count = 0;
    UINT16 crc16 = 0;
    UINT8 id[4] = {0};
#ifdef RF_CHANING_MODE
    UINT8 *data = NULL;
#else
    UINT8 data[SIZE_ESL_DATA_BUF] = {0};
    uint32_t rf_time = 0;
#endif
    UINT8 data_len = 0;
    UINT8 power = 0;
    UINT8 channel = 0;
    UINT16 datarate = 0;
    UINT8 duration = 0;
    UINT8 slot_duration = 0;

    UINT16 interval = 0;
    UINT8 mode = 0;
    RF_EventMask result;
    uint8_t  pend_flg = PEND_STOP;
#ifdef RF_CHANING_MODE
    send_chaningmode_init();
    write2buf = listInit();
#endif

    pdebug("wkup addr=0x%08X, len=%d\r\n", addr, len);

    if((addr==0) || (len==0))
    {
        ret = -1;
        goto done;
    }

    if(g3_get_wkup_para(addr, &datarate, &power, &duration, &slot_duration, &mode) == 0)
    {
        perr("g3_wkup() get para from flash.\r\n");
        ret = -2;
        goto done;
    }

    interval = g3_get_wkup_interval(addr);
    if(interval == 0)
    {
        interval = 30;
    }

    pdebug("wkup para: datarate=%d, power=%d, duration=%d, slot_duration=%d, interval=%d\r\n", \
            datarate, power, duration, slot_duration, interval);

    if(duration == 0)
    {
        pdebug("warning: wkup duration is 0\r\n");
        goto done;
    }
#ifdef RF_CHANING_MODE
    data = ((MyStruct*)write2buf)->tx->pPkt;
#endif
    addr += OFFSET_WKUP_DATA;
    if(get_one_data(addr, id, &channel, &data_len, data, sizeof(data)) == 0)
    {
        perr("g3_wkup() get data from flash.\r\n");
        ret = -3;
        goto done;
    }

    pdebug("wkup data: id:0x%02X-0x%02X-0x%02X-0x%02X, channel=%d, len=%d, data=", \
            id[0], id[1], id[2], id[3], channel, data_len);
    pdebughex(data, data_len);


    set_power_rate(power, datarate);
    set_frequence(channel);
#ifdef RF_CHANING_MODE
#else
    send_data_init(id, data, data_len, 5000);
    rf_time = RF_getCurrentTime();
#endif
    if(mode == 1)
    {
        duration_ms = (uint32_t)duration * 10;
    }
    else
    {
        duration_ms = (uint32_t)duration * 1000 - 500;
    }

    if((timer=TIM_Open(slot_duration, duration_ms/slot_duration, TIMER_DOWN_CNT, TIMER_PERIOD)) == TIMER_UNKNOW)
    {
        perr("g3_wkup() open timer.\r\n");
        ret = -4;
        goto done;
    }
    LED_TOGGLE(DEBUG_IO0);
    interval = EasyLink_us_To_RadioTime(interval);
    while(TIME_COUNTING==TIM_CheckTimeout(timer))
    {
        if(Core_GetQuitStatus() == 1)
        {
            pdebug("g3_wkup quit\r\n");
            break;
        }
        timer_count = TIM_GetCount(timer);
#ifdef RF_CHANING_MODE
        data = ((MyStruct*)write2buf)->tx->pPkt;
#endif
        data[1]  = (data[1]&0xC0) | ((timer_count>>8)&0x3f);
        data[2]  = timer_count & 0xff;

        /* calc crc */
        crc16 = CRC16_CaculateStepByStep(0, data, 4);
        crc16 = CRC16_CaculateStepByStep(crc16, id, 4);
        memcpy(data+4, &crc16, sizeof(crc16));
        pdebughex(data, data_len);

        LED_ON(DEBUG_IO1);
#ifdef RF_CHANING_MODE
        if (PEND_STOP == pend_flg){
            pend_flg = PEND_START;
            result = send_chaningmode(id, data, data_len, 6000);
            memcpy(((MyStruct*)write2buf->next)->tx->pPkt, data, data_len);
            write2buf = List_next(write2buf);
        }else{
            RF_wait_cmd_finish();
        }
#else
        rf_time += interval;
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
    LED_TOGGLE(DEBUG_IO0);
    TIM_Close(timer);
    ret = 1;
done:
    return ret;
}


#else
#undef RF_CHANING_MODE     //RF_CHANING_MODE can't work, need debug
#define GEO_ESL_UPLINK_MODE 0x40		//价签以上行链路的方式上报心跳。
#define GEO_ESL_HEARTBEAT_MODE		0x00
#define GEO_ESL_MODE_MASK			0X40
INT32 multi_set_wakeup_start(UINT32 addr, UINT32 len)
{
    INT32 ret = 0;
    UINT8 timer = 0;
    UINT32 duration_ms = 0;
    UINT16 timer_count = 0;
    UINT16 num = 0, i = 0, crc16 = 0;
    UINT8	real_num;
	UINT8	send_num;
    UINT8 id[4] = {0};
#ifdef RF_CHANING_MODE
    UINT8 *data = NULL;
#else
    UINT8 data[SIZE_ESL_DATA_BUF] = {0};
    uint32_t rf_time = 0;
#endif
    UINT8 data_len = 0;
    UINT8 power = 0;
    UINT8 channel = 0;
    UINT8 set_sn = 0;
    UINT16 datarate = 0;
    UINT8 duration = 0;
    UINT8 slot_duration = 0;
    UINT16 interval = 0;
    UINT8 mode = 0;
    RF_EventMask result;
    uint8_t  pend_flg = PEND_STOP;
    UINT32 tmpaddr=0;
    UINT8 set_bitmap = 0;


#ifdef RF_CHANING_MODE
    send_chaningmode_init();
    write2buf = listInit();
#endif
    pdebug("multi_set_wakeup_start addr=0x%08X, len=%d\r\n", addr, len);

    if((addr==0) || (len==0))
    {
        ret = -1;
        goto done;
    }

    if(g3_get_wkup_para(addr, &datarate, &power, &duration, &slot_duration, &mode) == 0)
    {
        perr("multi_set_wakeup_start() get para from flash\r\n");
        ret = -2;
        goto done;
    }

    interval = g3_get_wkup_interval(addr);
    if(interval == 0)
    {
        interval = 30;
    }

    num = g3_get_wkup_num(addr);

    if(duration == 0)
    {
        pdebug("warning: wkup duration is 0\r\n");
        goto done;
    }

    pdebug("multi_set_wakeup_start para: datarate=%d, power=%d, duration=%d, slot_duration=%d, mode = %d, interval=%d, num=%x\r\n", \
            datarate, power, duration, slot_duration, mode, interval, send_num);

    if (0 == (num&0xFF00)){
		real_num = num&0xFF;
		send_num = 4;;				 //send number is always 4.
		duration_ms = duration * 1000;
    }else{
    	real_num = num>>8;
		send_num = num&0xFF;
		duration_ms = duration * 1000 * send_num;
    }

    tmpaddr = addr + OFFSET_WKUP_DATA;
    for (i=0; i< real_num; i++)
    {
        set_bitmap |= 1 << get_location_set_num(tmpaddr, &data_len);
        tmpaddr += sizeof(id) + sizeof(channel) + sizeof(data_len) + sizeof(set_sn) +data_len;
    }

    set_power_rate(power, datarate);
//    set_frequence(channel);

    addr += OFFSET_WKUP_DATA;
    for(i = 0; i < send_num; i++)
    {
#ifdef RF_CHANING_MODE
        data = ((MyStruct*)write2buf)->tx->pPkt;
#endif
        if((set_bitmap & 1<<i) && (get_one_location_data(addr, id, &channel, &set_sn, &data_len, data, sizeof(data))==0))
        {
            perr("g3_wkup() get data from flash.\r\n");
            ret = -3;
            goto done;
        }
        set_frequence(channel);
#ifdef RF_CHANING_MODE
#else
        send_data_init(id, data, data_len, 5000);
        rf_time = RF_getCurrentTime();
#endif
        pdebug("wkup data: id:0x%02X-0x%02X-0x%02X-0x%02X, channel=%d, len=%d, data=", \
                id[0], id[1], id[2], id[3], channel, data_len);
        pdebughex(data, data_len);

        if((timer=TIM_Open(slot_duration, duration_ms/slot_duration/send_num, TIMER_DOWN_CNT, TIMER_PERIOD)) == TIMER_UNKNOW)
        {
            perr("g3_wkup() open timer\r\n");
            ret = -4;
            goto done;
        }

        interval = EasyLink_us_To_RadioTime(interval);
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
            timer_count = TIM_GetCount(timer) + (duration_ms/slot_duration/send_num)*(send_num-i-1);
            /* set slot time 在6.3.4中GEO_ESL_UPLINK_MODE位写错，应为0x80.但为了兼容，第7bit和第8bit互换。*/
            data[1]  = (data[1] & GEO_ESL_UPLINK_MODE) | GEO_ESL_UPLINK_MODE;
            data[1]  = data[1] | ((timer_count>>7)&0x80) | ((timer_count>>8)&0x3f);
            data[2]  = timer_count & 0xff;

            /* calc crc */
            crc16 = CRC16_CaculateStepByStep(0, data, 4);
            crc16 = CRC16_CaculateStepByStep(crc16, id, 4);
            memcpy(data+4, &crc16, sizeof(crc16));
            //pdebughex(data, data_len);

#ifdef RF_CHANING_MODE
            if (PEND_STOP == pend_flg){
                pend_flg = PEND_START;
                result = send_chaningmode(id, data, data_len, 6000);
                memcpy(((MyStruct*)write2buf->next)->tx->pPkt, data, data_len);
                write2buf = List_next(write2buf);
            }else{
                RF_wait_cmd_finish();
                //write2buf = List_next(write2buf);
            }
#else
            rf_time += interval;
            if (set_bitmap & 1<<i) {
                if (PEND_START == pend_flg){
                    send_pend(result);
                }

                result = send_async(rf_time);   //rf_time not use
                pend_flg = PEND_START;
            }
#endif


        }
#ifdef RF_CHANING_MODE
        RF_wait_cmd_finish(); //Wait for the last packet to be sent
        RF_cancle(result);
#endif
        TIM_Close(timer);

        if(Core_GetQuitStatus() == 1)
        {
            pdebug("g3_wkup quit2\r\n");
            break;
        }

        if (set_bitmap & 1<<i)
        {
            addr += sizeof(id) + sizeof(channel) + sizeof(data_len) + sizeof(set_sn) +data_len;
        }
    }

    ret = 1;
done:
    return ret;
}
#endif

INT32 wakeup_get_loop_times(UINT32 addr)
{
	return g3_get_wkup_loop_times(addr);
}
