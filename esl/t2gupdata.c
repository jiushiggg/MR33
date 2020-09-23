/*
 * t2gupdata.c
 *
 *  Created on: 2018��12��4��
 *      Author: ggg
 */
#include <stdint.h>
#include <string.h>
#include <ti\drivers\dpl\HwiP.h>

#include "cc2640r2_rf.h"
#include "updata1.h"
#include "updata0.h"
#include "frame1.h"
#include "timer.h"
#include "bsp.h"
#include "storage.h"
#include "debug.h"
#include "data.h"
#include "common.h"
#include "sleep.h"
#include "sys_cfg.h"
#include "core.h"
#include "stddef.h"
#include "t2gupdata.h"

INT32 t2g_send_sleep(updata_table_t *table, UINT8 timer);

UINT16 t2g_init_data(UINT32 addr, UINT32 len, updata_table_t *table)
{
    INT32 left_len = len;
    UINT16 one_packet_len = 0;
    UINT32 cur_addr = addr;
    UINT8 ff = 1;
    UINT8 last_id[4] = {0};
    UINT8 cur_id[4] = {0};
    INT32 i,j;
    UINT16 sn;

    mode1_esl_t *pESL = (mode1_esl_t *)table->data;
    table->max_esl_num = 1;

    pdebug("m1_init_data\r\n");

    table->esl_num = 0;
    table->pkg_num = 0;
    while(left_len > 0)
    {
        pdebug("%d,", left_len);

        one_packet_len = get_one_data(cur_addr, cur_id, NULL, NULL, NULL, 0);
        if(one_packet_len == 0)
        {
            perr("m1_init_data() get data!\r\n");
            break;
        }

        if((ff==1) || (memcmp(cur_id, last_id, 4) != 0)) // a new id
        {
			if(table->esl_num >= table->max_esl_num)
			{
				perr("m1_init_data() esl num(d%) > max(%d)!\r\n", table->esl_num, table->max_esl_num);
				break;
			}
            ff = 0;
            memcpy(last_id, cur_id, 4);
            memcpy(pESL[table->esl_num].esl_id, cur_id, 4);
            pESL[table->esl_num].first_pkg_addr = cur_addr;
            pESL[table->esl_num].failed_pkg_offset= 0;
            pdebug("\r\ntable %d: 0x%02X-0x%02X-0x%02X-0x%02X, first addr = 0x%08X.\r\n", table->esl_num, \
                    pESL[table->esl_num].esl_id[0], pESL[table->esl_num].esl_id[1], pESL[table->esl_num].esl_id[2], pESL[table->esl_num].esl_id[3], \
                    pESL[table->esl_num].first_pkg_addr);
            pESL[table->esl_num].ack = 0;
            pESL[table->esl_num].total_pkg_num = 0;
            pESL[table->esl_num].failed_pkg_num = 0;
            pESL[table->esl_num].sleep_flag = SLEEP_FRAME_CNT;
            table->esl_num++;
        }

        pESL[table->esl_num-1].total_pkg_num += 1;
        pESL[table->esl_num-1].failed_pkg_num += 1;
        table->pkg_num += 1;

        cur_addr += one_packet_len;
        left_len -= one_packet_len;
    }

    /* init failed pkg sn */
    pESL = (mode1_esl_t *)table->data;
    for(i = 0; i < table->esl_num; i++)
    {
        memset(pESL[i].failed_pkg, 0, T2G_MAX_FAILED_PKG_NUM*2);
        for(j = 0; (j<T2G_MAX_FAILED_PKG_NUM)&&(j < pESL[i].total_pkg_num); j++)
        {
            sn = get_pkg_sn_f(pESL[i].first_pkg_addr+j*sizeof(t2g_seg_data_t), offsetof(t2g_seg_data_t, packet_num));
            memcpy(pESL[i].failed_pkg+j*2, &sn, sizeof(sn));
        }
    }

    pdebug("\r\n");

    return table->esl_num;
}




static UINT8 query_miss_slot = 0;
static  UINT8 first_pkg_data[SIZE_ESL_DATA_BUF] = {0};

static INT32 t2g_query_miss(updata_table_t *table, UINT8 timer)
{
    INT32 ret = 0;
    UINT8 data[SIZE_ESL_DATA_BUF] = {0};
    UINT8 rxbuf[SIZE_ESL_DATA_BUF] = {0};
    INT32 i;
    UINT32 deal_timeout = table->deal_duration*1000;
    UINT8 channel = 0;

    mode1_esl_t *pESL = (mode1_esl_t *)table->data;

//  set_power_rate(table->tx_power, table->tx_datarate);

    pdebug("m1_query_miss, timer is %d.\r\n", timer);

    for(i = 0; i < table->esl_num; i++, query_miss_slot++)
    {
        if(Core_GetQuitStatus() == 1)
        {
            pdebug("m1_query_miss quit\r\n");
            break;
        }

        if(TIM_CheckTimeout(timer) == TIME_OUT)
        {
            pdebug("mode0_query_miss_round, timer timeout.\r\n");
            break;
        }

        if(pESL[i].failed_pkg_num == 0)
        {
            continue;
        }

        if(query_miss_slot == 0)
        {
            query_miss_slot = 1;
        }

        set_power_rate(table->tx_power, table->tx_datarate);
//      channel = g3_get_channel(pESL[i].first_pkg_addr);
        get_one_data(pESL[i].first_pkg_addr, NULL, &channel, NULL, first_pkg_data, sizeof(first_pkg_data));
        memset(data, 0, sizeof(data));
        t2g_make_link_query(pESL[i].esl_id, get_pkg_sn_f(pESL[i].first_pkg_addr+(pESL[i].total_pkg_num-1)*sizeof(t2g_seg_data_t), \
                                 offsetof(t2g_seg_data_t, packet_num)), \
                                query_miss_slot, first_pkg_data, data, sizeof(data));
        set_frequence(channel);
        pdebughex(data, sizeof(data));
        send_data(pESL[i].esl_id, data, sizeof(data), 2000);
//      exit_txrx();
        set_power_rate(RF_DEFAULT_POWER,table->rx_datarate);
        set_frequence(channel);
        memset(rxbuf, 0, sizeof(rxbuf));
        if(recv_data(table->master_id, rxbuf, sizeof(rxbuf), deal_timeout) == 0)
        {
            pdebug("recv timeout.\r\n");
            continue;
        }
        pdebug("recv:");
//        Debug_SetLevel(DEBUG_LEVEL_DEBUG);
        pdebughex(rxbuf, sizeof(rxbuf));
        Debug_SetLevel(DEBUG_LEVEL_INFO);
        ret++;

        if(t2g_check_link_query(pESL[i].esl_id, pESL[i].total_pkg_num, query_miss_slot, first_pkg_data, rxbuf, sizeof(rxbuf)) == 0)
        {
            pdebug("data check error.\r\n");
            continue;
        }
        pESL[i].failed_pkg_num = ((st_t2g_esl_rsp*)rxbuf)->lost_num;
        //memcpy((UINT8 *)&pESL[i].failed_pkg_num, &((st_t2g_esl_rsp*)rxbuf)->lost_num, sizeof(pESL[i].failed_pkg_num));
        memcpy((UINT8 *)pESL[i].failed_pkg, ((st_t2g_esl_rsp*)rxbuf)->lost_packet, T2G_MAX_FAILED_PKG_NUM*2);
        //memset((UINT8 *)pESL[i].failed_pkg, 0 , MAX_FAILED_PKG_NUM*2);
        pESL[i].failed_pkg_num &= MASK_OF_PKG_SN;
        if(pESL[i].failed_pkg_num == 0) // miss 0 pkg, tx successfully
        {
            //pESL[i].failed_pkg_num = 0;
            pESL[i].ack = ((st_t2g_esl_rsp*)rxbuf)->lost_packet[0];
#ifdef DEBUG_5
            if(pESL[i].ack != 0x40)
            {
                perr("%02X-%02X-%02X-%02X: %02X\r\n",
                        pESL[i].esl_id[0], pESL[i].esl_id[1], pESL[i].esl_id[2], pESL[i].esl_id[3], pESL[i].ack);
                perr("q: ");
                perrhex(data, sizeof(data));
                perr("r: ");
                perrhex(rxbuf, sizeof(rxbuf));
            }
            else
            {
                pinfo("%02X-%02X-%02X-%02X: 0x40\r\n",
                        pESL[i].esl_id[0], pESL[i].esl_id[1], pESL[i].esl_id[2], pESL[i].esl_id[3]);
            }
#endif
        }
        else if(pESL[i].failed_pkg_num == pESL[i].total_pkg_num)
        {
            pESL[i].failed_pkg_offset = 0;
        }
        else // > 0 && < total_pkg_num
        {
            pESL[i].failed_pkg_num = pESL[i].failed_pkg_num <= T2G_MAX_FAILED_PKG_NUM ? pESL[i].failed_pkg_num : T2G_MAX_FAILED_PKG_NUM;
        }

        pdebug("failed pkg num = %d\r\n", pESL[i].failed_pkg_num);
    }

//  dummy(table, table->tx_duration);
//  exit_txrx();
    wait(2000);

    return ret;
}

#define RF_CHANING_MODE
void t2g_transmit(updata_table_t *table, UINT8 timer)
{
	INT32 i = 0, j = 0, k = 0, t = 0;
	UINT8 id[4] = {0};
	UINT8 ch = 0;
	UINT8 len = 0;
	UINT8 data[SIZE_ESL_DATA_BUF] = {0};
	INT32 left_pkg_num = 0;
	INT32 dummy_us = 0;
	mode1_esl_t *pESL = (mode1_esl_t *)table->data;
	UINT16 tsn = 0;
	UINT32 taddr = 0;
	UINT8 f = 0;
    uint16_t result=0;
    UINT8 rf_flg = RF_IDLE;
    uint16_t  key;

#ifdef RF_CHANING_MODE
    send_chaningmode_init();
    write2buf = listInit();
#endif
	set_power_rate(table->tx_power, table->tx_datarate);

	for(i = 0; i < table->esl_num; i++)
	{
		left_pkg_num += pESL[i].failed_pkg_num;
	}
	pinfo("num %d\r\n", left_pkg_num);
	i = 0;
	j = 0;
	while(left_pkg_num > 0)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("m1_transmit quit\r\n");
			break;
		}

		if(TIM_CheckTimeout(timer) == TIME_OUT)
		{
			pdebug("timeout.\r\n");
			break;
		}

		if((pESL[i].failed_pkg_num != 0) && (j < pESL[i].failed_pkg_num))
		{
			if(pESL[i].failed_pkg_num == pESL[i].total_pkg_num)
			{
				taddr = pESL[i].first_pkg_addr + j*SIZE_ESL_DATA_SINGLE;
//				pdebug("pESL[i].first_pkg_addr%x\r\n", pESL[i].first_pkg_addr);
				pdebug("send 0x%02X-0x%02X-0x%02X-0x%02X pkg %d, ch=%d, len=%d\r\n", id[0], id[1], id[2], id[3], j, ch, len);
			}
//			else if((pESL[i].failed_pkg_num>=MAX_FAILED_PKG_NUM) && (pESL[i].failed_pkg_num<pESL[i].total_pkg_num)) // >= 10 && < total_pkg_num
//			{
//				taddr = pESL[i].first_pkg_addr + (pESL[i].failed_pkg_offset+j)*SIZE_ESL_DATA_SINGLE;
//				pdebug("send misss 0x%02X-0x%02X-0x%02X-0x%02X pkg %d, ch=%d, len=%d\r\n", id[0], id[1], id[2], id[3], j, ch, len);
//			}
			else // > 0 && < 10
			{

				tsn = get_missed_sn_r(pESL[i].failed_pkg, j);
				pinfo("lost packet:%d\r\n", tsn);
				//for debug
//				search_pkg_sn_times = 0;
//				memset(search_pkg_history, 0, sizeof(search_pkg_history));
//				memset(search_addr_history, 0, sizeof(search_addr_history));
//				search_end_pkg = 0;
//				memset(search_pkg_history_o, 0, sizeof(search_pkg_history_o));
//				memset(search_first_pkg, 0, sizeof(search_first_pkg));

				taddr = get_pkg_addr_bsearch(pESL[i].first_pkg_addr, pESL[i].total_pkg_num, tsn, offsetof(t2g_seg_data_t, packet_num));
				if(taddr == 0)
				{
					pinfo("m1t can't find %02X-%02X-%02X-%02X pkg %d 0x%08X, 0x%08X, %d\r\n",
						pESL[i].esl_id[0],pESL[i].esl_id[1],pESL[i].esl_id[2],pESL[i].esl_id[3],
						tsn, taddr, pESL[i].first_pkg_addr, pESL[i].total_pkg_num);

					//for debug
//					perr("search end pkg: %d\r\n", search_end_pkg);
//					perrhex((UINT8 *)search_pkg_history, search_pkg_sn_times*2);
//					perrhex((UINT8 *)search_addr_history, search_pkg_sn_times*4);
//					perrhex((UINT8 *)search_pkg_history_o, search_pkg_sn_times*2);
//					perrhex((UINT8 *)search_first_pkg, 32);

					left_pkg_num -= pESL[i].failed_pkg_num-j-1;
					pESL[i].failed_pkg_num = 0;
					pESL[i].ack = 0x5F;

					goto user_continue;
				}
//				pESL[i].failed_pkg_offset = (taddr-pESL[i].first_pkg_addr)/SIZE_ESL_DATA_SINGLE + 1;
				pinfo("send miss 0x%02X-0x%02X-0x%02X-0x%02X pkg %d, ch=%d, len=%d\r\n", id[0], id[1], id[2], id[3], tsn, ch, len);
			}

 			if(get_one_data(taddr, id, &ch, &len, data, SIZE_ESL_DATA_BUF) == 0)
			{
				perr("m1_transmit() get data!\r\n");
				goto user_continue;
			}
 			pdebug("send 0x%02X-0x%02X-0x%02X-0x%02X pkg %d, ch=%d, len=%d\r\n", id[0], id[1], id[2], id[3], j, ch, len);
#ifdef RF_CHANING_MODE
 			key = HwiP_disable();
            memcpy(((MyStruct*)write2buf)->tx->pPkt, data ,SIZE_ESL_DATA_BUF);
            ((MyStruct*)write2buf)->tx->syncWord = ((uint32_t)id[0]<<24) | ((uint32_t)id[1]<<16) | ((uint32_t)id[2]<<8) | id[3];
            HwiP_restore(key);
#endif
 			pdebughex(data, len);

#ifdef RF_CHANING_MODE
			if (RF_IDLE == rf_flg){
			    rf_flg = RF_WORKING;
			    set_frequence(ch);
			    memcpy(((MyStruct*)write2buf->next)->tx->pPkt, data ,SIZE_ESL_DATA_BUF);
                result = send_chaningmode(id, data, len, 6000);
                write2buf = List_next(write2buf);
			}else{
			    RF_wait_cmd_finish();
			}

#else
            if (PEND_START == rf_flg){
                send_pend(result);
            }
            result = send_without_wait(id, data, len, ch, 6000);
            rf_flg = PEND_START;
#endif
		user_continue:
//		    write2buf = List_next(write2buf);
			left_pkg_num--;
			k++;
			t++;
		}

		f = 0;
		i++;
		if(i == table->esl_num)
		{
		    //pinfo("%d.",table->tx_interval);       //debug
			if((dummy_us=((uint16_t)table->tx_interval*1000-k*table->tx_duration))>=0 && rf_flg==RF_WORKING)
			{
#ifdef RF_CHANING_MODE
                dummy_chaining_mode(table, dummy_us);
#else
                dummy(table, dummy_us);
#endif
				f = 1;
			}
            i = k = 0;
            j++;
		}
		if((t%100==0) && (t<ESL_REC_FRAME1_TIMEOUT) && (f==0)&& rf_flg==RF_WORKING)
		{
#ifdef RF_CHANING_MODE
                dummy_chaining_mode(table, table->tx_duration);
#else
                dummy(table, table->tx_duration);
#endif
		}
	}
#ifdef RF_CHANING_MODE
	RF_wait_cmd_finish(); //Wait for the last packet to be sent
	RF_cancle(result);
#endif
//	if((i != table->esl_num) && (f == 0))           //����ֲ�ĳ���û�б�Ҫ�ٷ���֡�������������㹻��ʱ�俪�����ղ�ѯ��
//	{
//		if((dummy_us=(table->tx_interval*1000-k*table->tx_duration)) >= 0)
//		{
//			dummy(table, dummy_us);
//			f = 1;
//		}
//		else
//		{
//			dummy(table, table->tx_duration);
//		}
//	}

	wait(2000);
}
UINT8 t2g_updata_loop(updata_table_t *table)
{
    UINT8 ret = 0;
    UINT8 timer = 0;
    UINT16 timeout = table->esl_work_duration * 10 * 85 / 100;
    UINT16 leftime = 0;

    pdebug("t2g_updata_loop().\r\nfirst rount timeout is %d.\r\n", timeout);
    if((timer=TIM_Open(100, timeout, TIMER_UP_CNT, TIMER_PERIOD)) == TIMER_UNKNOW)
    {
        goto done;
    }
    t2g_transmit(table, timer);
    leftime = timeout - TIM_GetCount(timer);
    TIM_Close(timer);
    if(Core_GetQuitStatus() == 1)
    {
        pinfoEsl("t2g_updata_loop quit\r\n");
        pdebug("t2g_updata_loop quit\r\n");
        goto done;
    }
    timeout = table->esl_work_duration * 10 * 15 / 100 + leftime;
    pdebug("first round left %d ms.\r\nsecond timeout is %d.\r\n", leftime, timeout);
    if((timer=TIM_Open(100, timeout, TIMER_UP_CNT, TIMER_ONCE)) == TIMER_UNKNOW)
    {
        goto done;
    }
    while(1)
    {
        t2g_query_miss(table, timer);
        if(TIM_CheckTimeout(timer) == TIME_OUT)
        {
            break;
        }
        if(Core_GetQuitStatus() == 1)
        {
            pdebug("m1_updata_loop quit1\r\n");
            break;
        }
        t2g_send_sleep(table, timer);
        //if(table->ok_esl_num==table->esl_num ||  table->esl_num==(table->ok_esl_num/SLEEP_FRAME_CNT))
        if(table->ok_esl_num==table->esl_num)
        {
            break;
        }
        if(TIM_CheckTimeout(timer) == TIME_OUT)
        {
            break;
        }
        if(Core_GetQuitStatus() == 1)
        {
            pdebug("m1_updata_loop quit2\r\n");
            break;
        }


        t2g_transmit(table, timer);
        if(TIM_CheckTimeout(timer) == TIME_OUT)
        {
            break;
        }
        if(Core_GetQuitStatus() == 1)
        {
            pdebug("m1_updata_loop quit3\r\n");
            break;
        }
    }

    ret = 1;
    TIM_Close(timer);
//    t2g_send_sleep(table, timer);
done:

    return ret;
}

INT32 t2g_send_sleep(updata_table_t *table, UINT8 timer)
{
    UINT8 data[SIZE_ESL_DATA_BUF] = {0};
    UINT32 i;
    INT32 ret = 0;
    UINT8 channel = 0;
    volatile INT8 prev_channel=RF_FREQUENCY_UNKNOW;
    mode1_esl_t *pESL = (mode1_esl_t *)table->data;

    pdebug("mode1_send_sleep(), timer: %d.\r\n", timer);

    set_power_rate(table->tx_power, table->tx_datarate);

    for(i = 0; i < table->esl_num; i++)
    {
        if(Core_GetQuitStatus() == 1)
        {
            pdebug("m1_send_sleep quit\r\n");
            break;
        }

        if(TIM_CheckTimeout(timer) == TIME_OUT)
        {
            break;
        }

        if((pESL[i].failed_pkg_num==0) && (pESL[i].sleep_flag>0))
        {
            pdebug("sp %02X%02X%02X%02X\r\n", \
                    pESL[i].esl_id[0], pESL[i].esl_id[1], \
                    pESL[i].esl_id[2], pESL[i].esl_id[3]);
            pESL[i].sleep_flag--;

            t2g_make_sleep_data(pESL[i].esl_id, table->id_x_ctrl, data, sizeof(data));
            get_one_data(pESL[i].first_pkg_addr, NULL, &channel, NULL, NULL, 0);

            if (channel != prev_channel){
                set_frequence(channel);
            }
            prev_channel = channel;
            send_data(pESL[i].esl_id, data, sizeof(data), 2000);
            //send_data(pESL[i].esl_id, data, sizeof(data), channel, 6000);
            if (pESL[i].sleep_flag == 0){
                ret++;
            }
        }
    }

    table->ok_esl_num += ret;
    return ret;
}

void t2g_make_new_ack(updata_table_t *table, UINT32 *ack_addr, UINT32 *ack_len, UINT8 sn_offset)
{
    UINT32 offset = 0;
    INT32 i;
    UINT8 ack = 0;
    mode1_esl_t *pESL = (mode1_esl_t *)table->data;

    pdebug("m1_make_new_ack\r\n");
    // one ack : 4B ID + 1B ACK + 20B DETAIL, other: 4B sid, 2B cmd, 4B cmdlen, 1B para, 8B rev, 2B num, 2B crc
    *ack_len = table->esl_num*(4+1+20) + 4 + 19;

    *ack_addr =(UINT32)storage_malloc(*ack_len);
    if(*ack_addr == NULL)
    {
        perr("m1_make_new_ack malloc addr = 0x%08X, len = %d!\r\n", \
             *ack_addr, *ack_len);

        *ack_len = 0;
        goto done;
    }
    else
    {
        pdebug("m1_make_new_ack malloc addr = 0x%08X, len = %d, success.\r\n", \
               *ack_addr, *ack_len);
    }

    offset = *ack_addr;
    if(g3_set_ack_para(offset, table->sid, CMD_ACK_NEW, table->esl_num*(4+1+20)+11, 0, table->esl_num) == 0)
    {
        perr("m1_make_new_ack set ack para!\r\n");
        *ack_len = 0;
        goto done;
    }
    offset += 21;

    for(i = 0; i < table->esl_num; i++)
    {
        if(pESL[i].failed_pkg_num == 0) //ͨѶ�ϣ����а������ɹ�
        {
            ack = pESL[i].ack == 1 ? 2 : pESL[i].ack; // new ack
        }
        else //ͨѶ�ϣ��ж�����������С��10
        {
            ack = 0;
        }

        if(g3_set_new_ack(offset, pESL[i].esl_id, ack, pESL[i].failed_pkg, T2G_MAX_FAILED_PKG_NUM*2) == 0)
        {
            perr("m1_make_new_ack set ack!\r\n");
            *ack_len = 0;
            goto done;
        }
        else
        {
            offset += 4+1+20;
        }
    }

    if(g3_set_ack_crc(*ack_addr, *ack_len-2) == 0)
    {
        perr("m1_make_new_ack set crc!\r\n");
        *ack_len = 0;
    }

done:
    return;
}
