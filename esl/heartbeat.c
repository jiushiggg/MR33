#include <ti/sysbios/BIOS.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "heartbeat.h"
#include "cc2640r2_rf.h"
#include "timer.h"
#include "bsp.h"
#include "debug.h"
#include "crc16.h"
#include "core.h"
#include "tap2go.h"
#include "event.h"
#include "watchdog.h"

#define NORMAL_DATA     0
#define ESL_UPLINK_DATA 1
#define SURVEY_DATA     2
#define RC_UPLINK_DATA	4
#define ERR_DATA        0x80

#define HEARTBEAT_ONE_TIMES		1


#define CRC_ERR             0x40

#define ESL_HB_PERIOD		600				//600S, ESL heartbeat period 180s

#pragma pack(1)
typedef struct survey_esl{
    uint8_t ctrl;
    uint8_t reserved[4];
    uint16_t ap_id;
    uint8_t ap_rf_id[4];
    uint16_t ap_data_rate;
    uint8_t ap_power;
    uint8_t ap_ch;
    uint8_t esl_rf_id[4];
    uint16_t esl_data_rate;
    uint8_t esl_power;
    uint8_t esl_ch;
    uint8_t round;
    uint16_t crc;
}st_survey_esl;

typedef struct st_rc2ap{
    uint8_t ctrl;
    uint8_t rc_id[4];
    uint8_t type;
    uint8_t sn;
    uint8_t esl_id[4];
    uint8_t opt[5];
    uint8_t succ;
    uint16_t data;
    uint8_t reserved[5];
    uint16_t crc;
}st_rc2ap;

typedef struct st_ap2rc{
    uint8_t ctrl;
    uint8_t rc_id[4];
    uint16_t ap_num;
    uint8_t succ;
    uint8_t reserved[16];
    uint16_t crc;
}st_ap2rc;

typedef struct st_esl_uplink{
    uint8_t ctrl;
    uint8_t id[ID_LEN];
    uint8_t session_id;
    uint8_t default_ch;
    uint8_t req_type;
    st_req_content req;
}st_esl_uplink;

typedef struct HbStruct {
    List_Elem elem;
    st_hb *buf;
} HbStruct;

typedef struct HbUplinkStruct{
    List_Elem elem;
    uint8_t *buf;
} HbUplinkStruct;


typedef struct uplink_req{
    uint8_t ctrl;
    uint8_t id[ID_LEN];
    uint8_t session_id;
    uint8_t default_ch;
    uint8_t req_type;
    uint8_t req_data[16];
    uint16_t crc;
}uplink_req;

typedef struct st_upLink_buf{
	st_protcolHead head;
	uint8_t hb_buf[27];
}st_upLink_buf;

typedef struct
{
	UINT8 id[4];
	UINT8 sessionid;
	UINT8 index;
	UINT8 modby;
}esl_uplink_info_t;
#pragma pack()

static INT32 _esl_uplink_num = 0;
List_Elem *phb_list	= NULL;
List_List Hb_list;
HbStruct Hb_foo[2];

List_Elem *puplink_buf = NULL;
List_List Hb_upLinklist;
HbUplinkStruct Hb_upLinkfoo[UPLINK_BUF_NUM];
volatile int8_t  rf_normal_data;
volatile int8_t  rf_up_data;
volatile int8_t  rf_up_rc_data;


esl_uplink_info_t esl_uplink_info[UPLINK_BUF_NUM];

void TIM_heartbeatCallback(uint8_t n)
{
	RF_SemRecvPost();
	rf_normal_data = true;
}


//timeout: µ¥Î»s
UINT8 set_timer(INT32 timeout)
{
	UINT8 timer = 0;
	
	if(timeout != 0)
	{	
		if(timeout > 4294) //0xffFFffFF/1000000
		{
			timeout = 4294;
			pinfo("warning: set_timer timeout change to 4294.\r\n");
		}
		
		if((timer=TIM_Open(timeout*1000, 1, TIMER_UP_CNT, TIMER_ONCE)) == TIMER_UNKNOW)
		{
			perr("set_timer open.\r\n");
		}
		TIM_SetCallback(timer, TIM_heartbeatCallback);
	}
	else
	{
		timer = 0;
	}
	
	return timer;
}


/* 
len == 16
0xc0:g2;
0xf0:old g3;
0x50:old g3 firmware
len == 26
0x80: new e31
*/


static INT8 _check_hb_data(UINT8 *src, UINT8 len)
{
    UINT8 ctrl = 0;
    UINT8 id[4] = {0};
    UINT16 read_crc=0, cal_crc=0;
    if (CTRL_STREQ == src[0]){
        ctrl = src[0];
    }else if(CTRL_RCREQ == src[0]){
        ctrl = src[0];
    }else{
        ctrl = src[0] & 0xF0;
    }
    if(len == 16)
    {
        //need check crc
        memcpy(&read_crc, src+len-2, sizeof(read_crc));
        if(ctrl == 0xC0)
        {
            cal_crc = CRC16_CaculateStepByStep(cal_crc, src, len-2);
            if(cal_crc == read_crc)
            {
                return NORMAL_DATA;
            }
        }
		else if((ctrl==0xF0)||(ctrl==0x50)||(ctrl==0xE0)||(ctrl==0x10))
        {
            memcpy(id, src+5, sizeof(id));
            cal_crc = CRC16_CaculateStepByStep(cal_crc, src, len-2);
            cal_crc = CRC16_CaculateStepByStep(cal_crc, id, sizeof(id));
            if(cal_crc == read_crc)
            {
                return NORMAL_DATA;
            }
        }

    }
    else if(len == 26)
    {
        if(ctrl == CTRL_HEARTBEAT){
            return NORMAL_DATA;
        }else if (ctrl == CTRL_ESL_UPLINK_DATA){
            return ESL_UPLINK_DATA;
        }else if (ctrl == CTRL_RCREQ){
            return RC_UPLINK_DATA;
        }else if (ctrl == CTRL_STREQ){
            return SURVEY_DATA;
        }else {
            return ERR_DATA;
        }
    }

    return ERR_DATA;
}

static INT32 _compare_esl_uplink_info_by_index(const void *info1, const void *info2)
{
	esl_uplink_info_t *p1 = (esl_uplink_info_t *)info1;
	esl_uplink_info_t *p2 = (esl_uplink_info_t *)info2;
	return (p1->index - p2->index);
}

static void _sort_esl_uplink_info_by_index(esl_uplink_info_t *start, INT32 num)
{
	qsort(start, num, sizeof(esl_uplink_info_t), _compare_esl_uplink_info_by_index);
}

static INT32 _compare_esl_uplink_info_by_eslid(const void *info1, const void *info2)
{
	esl_uplink_info_t *p1 = (esl_uplink_info_t *)info1;
	esl_uplink_info_t *p2 = (esl_uplink_info_t *)info2;
	return memcmp(p1->id, p2->id, 4);
}

static esl_uplink_info_t *_get_esl_uplink_info(esl_uplink_info_t *start, INT32 num, UINT8 *eslid)
{
	esl_uplink_info_t key;
	memcpy(key.id, eslid, 4);
	qsort(start, num, sizeof(esl_uplink_info_t), _compare_esl_uplink_info_by_eslid);
	return (esl_uplink_info_t *)bsearch(&key, start, num, sizeof(esl_uplink_info_t), _compare_esl_uplink_info_by_eslid);
}

static esl_uplink_info_t *_append_esl_uplink_info(esl_uplink_info_t *start, INT32 *num, UINT8 *eslid, UINT8 sid)
{
	esl_uplink_info_t *pinfo = NULL;
	INT32 i;
	
	if(*num >= UPLINK_BUF_NUM)
	{
		_sort_esl_uplink_info_by_index(start, UPLINK_BUF_NUM);
		pinfo = start;
		memcpy(pinfo->id, eslid, 4);
		pinfo->sessionid = sid;
		pinfo->modby = 12;
		pinfo->index = UPLINK_BUF_NUM;
		for(i = 0; i < UPLINK_BUF_NUM; i++) //update the index
		{
			pinfo->index--;
			pinfo++;
		}
		
		return start;
	}
	else
	{
		pinfo = start + *num;
		memcpy(pinfo->id, eslid, 4);
		pinfo->sessionid = sid;
		pinfo->modby = 12;
		pinfo->index = *num;
		*num = *num + 1;

		return pinfo;
	}
}

static esl_uplink_info_t *_handle_esl_uplink_data(esl_uplink_info_t *start, INT32 *num, UINT8 *data, INT32 len)
{
	UINT8 *eslid = data+1;
	UINT8 sid = *(data+5);
	esl_uplink_info_t *pinfo = _get_esl_uplink_info(start, *num, eslid);
	
	if(pinfo != NULL)
	{
		if(pinfo->sessionid == sid)
		{
			pinfo->modby = pinfo->modby == 6 ? 12 : (pinfo->modby-1);
		}
		else
		{
			pinfo->sessionid = sid;
			pinfo->modby = 12;
		}
	}
	else
	{
		pinfo = _append_esl_uplink_info(start, num, eslid, sid);
	}
	
	log_print("uplink info @ 0x%08X, num = %d\r\n", (int)start, (int)*num);
	phex((UINT8 *)start, sizeof(esl_uplink_info_t)*UPLINK_BUF_NUM);
	
	return pinfo;
}

static void _ack_birang(INT32 apid, INT32 modby)
{
	BSP_Delay100US((apid%modby)*4);
}

static INT32 _ack_the_esl(UINT8 *uplink_data, INT32 len, UINT8 status)
{
	UINT8 *eslid = uplink_data+1;
	UINT8 eslch = *(uplink_data+6);
	UINT8 ack_buf[9] = {0};
	UINT16 crc = 0;
	
	ack_buf[0] = uplink_data[0]; //ctrl
	memcpy(ack_buf+1, eslid, 4);//id
	ack_buf[5] = *(uplink_data+5); //session id
	ack_buf[6] = status;
	crc = CRC16_CaculateStepByStep(crc, ack_buf, 7);
	crc = CRC16_CaculateStepByStep(crc, eslid, 4);
	memcpy(ack_buf+7, &crc, sizeof(crc));
	set_power_rate(RF_DEFAULT_POWER, DATA_RATE_500K);
    set_frequence(eslch);
    send_data(eslid, ack_buf, sizeof(ack_buf), 2000);
	//pinfo("_ack_the_esl %02X-%02X-%02X-%02X, %d: ", eslid[0], eslid[1], eslid[2], eslid[3], eslch);
	//phex(ack_buf, sizeof(ack_buf));
	return 0;
}

//#if defined(PCIE)
static INT32 _ack_the_rc(UINT8 *uplink_data, g3_hb_table_t *table)
{
    st_rc2ap *data = (st_rc2ap*)uplink_data;
    st_ap2rc ack_buf = {0};

    ack_buf.ctrl = data->ctrl + 1; //ctrl
    memcpy(ack_buf.rc_id, data->rc_id, sizeof(ack_buf.rc_id));//id
    ack_buf.ap_num = (uint8_t)table->apid;
    ack_buf.succ = 1;
    ack_buf.crc = CRC16_CaculateStepByStep(0, (uint8_t*)&ack_buf, sizeof(ack_buf)-sizeof(ack_buf.crc));

    set_power_rate(RF_DEFAULT_POWER, DATA_RATE_500K);
    set_frequence(table->channel);
    send_data(ack_buf.rc_id, (uint8_t*)&ack_buf, sizeof(ack_buf), 2000);

    return 0;
}
//#endif

extern uint8_t core_idel_flag;



List_Elem* Hb_listInit(g3_hb_table_t *table)
{
	Hb_foo[0].buf = &table->data_buf[0];
	Hb_foo[1].buf = &table->data_buf[1];

    List_clearList(&Hb_list);
    List_put(&Hb_list, (List_Elem *)&Hb_foo[0]);
    List_put(&Hb_list, (List_Elem *)&Hb_foo[1]);

    Hb_foo[1].elem.next = (List_Elem *)&Hb_foo[0];
    Hb_foo[0].elem.prev = (List_Elem *)&Hb_foo[1];


    return List_head(&Hb_list);
}

List_Elem* Hb_uplinkListInit(g3_hb_table_t *table)
{
	uint8_t i = 0;
	for (i=0; i < UPLINK_BUF_NUM; i++){
		Hb_upLinkfoo[i].buf = &table->uplink_buf[i][0];
	}

    List_clearList(&Hb_upLinklist);
    //List_put(&Hb_list, (List_Elem *)&Hb_foo[0]);
    return List_head(&Hb_upLinklist);
}

static INT32 _hb_recv(g3_hb_table_t *table, UINT8 (*uplink)(UINT8 *src, UINT32 len))
{
	uint16_t cmdHandle;
//	uint8_t hb_timer = 0;
	esl_uplink_info_t *pesluplinkinfo = NULL;
	st_upLink_buf tmp_buf;
	HbStruct *send_hb_buf;				//temporary pointer
	uint32_t SWIkey;
	phb_list = Hb_listInit(table);		//Initializes a circular list that has two elements, each element is a st_hb type
	puplink_buf = Hb_uplinkListInit(table);
	Bool ret = 0;
#if 0
	//52567853,ch=2,R=100,L=26,T=0,TO=30,LO=0,NO=500,loop=1
	table->id[0]=0x52;
	table->id[1]=0x56;
	table->id[2]=0x78;
	table->id[3]=0x53;
	table->channel=2;
	table->recv_bps=100;
	table->recv_len=26;
	table->interval=0;
	table->timeout=30;
	table->lenout=0;
	table->numout=500;
	table->loop=1;
#endif

	pinfo("%x%x%x%x,ch=%d,R=%d,L=%d,T=%d,TO=%d,LO=%d,apid=%d,loop=%d\r\n", \
			table->id[0], table->id[1], table->id[2], table->id[3], table->channel, table->recv_bps, \
			table->recv_len, table->interval, table->timeout, table->lenout, table->apid, table->loop);

	/* reset recv buf */
	memset((uint8_t*)table->data_buf, 0, sizeof(table->data_buf));

//	hb_timer = set_timer(table->timeout);
	table->timeout *= 1000*1000/Clock_tickPeriod;
	set_power_rate(RF_DEFAULT_POWER, table->recv_bps);
	set_frequence(table->channel);
	rf_preset_hb_recv(true);
	Semaphore_RFReconfig(SEMAPHORE_COUNTING);

	cmdHandle = RF_recvDataForever(table->id, table->recv_len);
	while(1)
	{
		ret = RF_SemRecvPend(table->timeout);
		watchdog_clear();
		if (false == ret){
			pinfo("timeout:%d\r\n", table->timeout);
			SWIkey = swiDisable();
        	phb_list = List_next(phb_list);
        	rf_normal_data = true;
        	swiRestore(SWIkey);
		}

		if(Core_GetQuitStatus() == 1){
			pinfo("back to idel\r\n");
			break;
		}
		//if (rf_up_data && table->apid!=0){
		if (rf_up_data){				//for test
			rf_up_data = false;
	        while( Hb_upLinklist.head != NULL) {
	        	pdebug("head address = 0x%x\r\n", Hb_upLinklist.head);
	        	pdebug("address->next = 0x%x\r\n", puplink_buf->next);
				pdebug("address->prev = 0x%x\r\n", puplink_buf->prev);

				SWIkey = swiDisable();
				puplink_buf = List_get(&Hb_upLinklist);
			    tmp_buf.head.len = sizeof(table->uplink_buf[0]);
		        memcpy(tmp_buf.hb_buf, ((HbUplinkStruct*)puplink_buf)->buf, tmp_buf.head.len);
		        swiRestore(SWIkey);

				//Debug_SetLevel(DEBUG_LEVEL_DEBUG);
				pdebughex(((HbUplinkStruct*)puplink_buf)->buf, 27);
				Debug_SetLevel(DEBUG_LEVEL_INFO);
			    uint8_t query_type_offset = offsetof(st_t2g_frame1, req_type);
			    switch(tmp_buf.hb_buf[query_type_offset]){
#if defined(TAP2GO)
					case UPLINK_T2G_MODE:
#endif
					case UPLINK_GEOLOCATION_HB:
				    	tmp_buf.head.cmd = CORE_CMD_TAG2GO_MESSAGE;
				        pdebug("t2gUp%x%x%x%x\r\n", tmp_buf.hb_buf[1], tmp_buf.hb_buf[2],tmp_buf.hb_buf[3],tmp_buf.hb_buf[4]);
						break;
					case UPLINK_BINDING_INT:
					case UPLINK_BINDING_STR:
					case UPLINK_UPDATE_INT:
					case UPLINK_UPDATE_STR:
						pesluplinkinfo = _handle_esl_uplink_data(esl_uplink_info, &_esl_uplink_num, tmp_buf.hb_buf, 0);//len is useless
						_ack_birang(table->apid, pesluplinkinfo->modby);
						RF_cancle(cmdHandle);
						_ack_the_esl(tmp_buf.hb_buf, 0, 0);	//len is useless
						set_power_rate(RF_DEFAULT_POWER, table->recv_bps);
						set_frequence(table->channel);
						cmdHandle = RF_recvDataForever(table->id, table->recv_len);
						tmp_buf.head.cmd = CORE_CMD_ESL_UPLINK_ACK;
						pinfo("eslUp\r\n");
						break;
					default:
						memset(&tmp_buf, 0, sizeof(tmp_buf));
						break;
			    }

			    if (0 == tmp_buf.head.cmd){
			    	continue;
			    }
		        uplink((uint8_t*)&tmp_buf, sizeof(st_protcolHead)+tmp_buf.head.len);

	        }
		}
//#if defined(PCIE)
		if (rf_up_rc_data){
		    rf_up_rc_data = false;
		    RF_cancle(cmdHandle);
		    while( Hb_upLinklist.head != NULL) {
                pdebug("head address = 0x%x\r\n", Hb_upLinklist.head);
                pdebug("address->next = 0x%x\r\n", puplink_buf->next);
                pdebug("address->prev = 0x%x\r\n", puplink_buf->prev);

                SWIkey = swiDisable();
                puplink_buf = List_get(&Hb_upLinklist);
                tmp_buf.head.len = sizeof(table->uplink_buf[0]);
                memcpy(tmp_buf.hb_buf, ((HbUplinkStruct*)puplink_buf)->buf, tmp_buf.head.len);
                swiRestore(SWIkey);

                BSP_Delay1MS(table->apid);
                _ack_the_rc(tmp_buf.hb_buf, table);
                tmp_buf.head.cmd = CORE_CMD_RC_UPLINK_ACK;
                //Debug_SetLevel(DEBUG_LEVEL_DEBUG);
                pdebughex(((HbUplinkStruct*)puplink_buf)->buf, 27);
                Debug_SetLevel(DEBUG_LEVEL_INFO);
                uplink((uint8_t*)&tmp_buf, sizeof(st_protcolHead)+tmp_buf.head.len);
		    }
		    set_power_rate(RF_DEFAULT_POWER, DATA_RATE_100K);
		    set_frequence(table->channel);
		    cmdHandle = RF_recvDataForever(table->id, table->recv_len);

		}
//#endif
		if (rf_normal_data){
			rf_normal_data = false;

			send_hb_buf = (HbStruct*)List_prev(phb_list);
			send_hb_buf->buf->head.cmd = CORE_CMD_HB_ACK;
			send_hb_buf->buf->head.len += sizeof(send_hb_buf->buf->num);

			uplink((uint8_t*)send_hb_buf->buf, sizeof(st_protcolHead)+send_hb_buf->buf->head.len);
//			pinfo("L£º%d,N£º%d\r\n", sizeof(st_protcolHead)+send_hb_buf->buf->head.len,
//				  	  	  	  	  	  	  	  	   send_hb_buf->buf->num);
//			pinfo("N:%d, len:%d\r\n", send_hb_buf->buf->num, send_hb_buf->buf->head.len);

			memset(send_hb_buf->buf , 0, sizeof(st_hb));
			if (HEARTBEAT_ONE_TIMES == table->loop){
				pinfo("send one times:%d\r\n",table->loop);
				break;
			}
		}
		if ((RF_cmdPropRxAdv.status&PROP_ERROR_TYPE) == PROP_ERROR_TYPE){
			pinfo("RF err:%x\r\n",RF_cmdPropRxAdv.status);
			//RF_cancle(cmdHandle);	//todo:close RF £¬ then open
			RF_closeRF();
			rf_init();
			table->timeout *= 1000*1000/Clock_tickPeriod;
			set_power_rate(RF_DEFAULT_POWER, table->recv_bps);
			set_frequence(table->channel);
			rf_preset_hb_recv(true);
			Semaphore_RFReconfig(SEMAPHORE_COUNTING);
			cmdHandle = RF_recvDataForever(table->id, table->recv_len);
		}
	}

//	TIM_Close(hb_timer);
//	TIM_SetCallback(hb_timer, NULL);
	if(Core_GetQuitStatus() == 1){
		Core_SendAck(0x10f0, 0, NULL);			//must return a data
	}
	exit_txrx();
	RF_cancle(cmdHandle);
	rf_preset_hb_recv(false);
	Semaphore_RFReconfig(SEMAPHORE_BINARY);
	
	return 0;
}

INT32 common_recv_parse_cmd(UINT8 *pCmd, INT32 cmdLen, g3_hb_table_t *table)
{
	UINT8 num = 0;
	UINT8 *ptr = pCmd;
	
	memcpy(table->id, ptr, 4);
	ptr += 4;
	num++;
	
	table->channel = *ptr;
	ptr += 1;
	num++;
	
	memcpy((UINT8 *)&table->recv_bps, ptr, 2);
	ptr += 2;
	num++;
	
	table->recv_len = *ptr;
	ptr += 1;
	num++;
	
	memcpy((UINT8 *)&table->interval, ptr, 4);
	ptr += 4;
	num++;

	memcpy((UINT8 *)&table->timeout, ptr, 4);
	ptr += 4;
	num++;

	memcpy((UINT8 *)&table->lenout, ptr, 4);
	ptr += 4;
	num++;

	memcpy((UINT8 *)&table->numout, ptr, 4);
	ptr += 4;
	num++;
	
	memcpy((UINT8 *)&table->loop, ptr, 4);
	ptr += 4;
	num++;
	
	memcpy((UINT8 *)&table->apid, ptr, 4);
	ptr += 4;
	num++;

//	pdebug("hb table data:");
//	pdebughex((UINT8 *)table, sizeof(g3_hb_table_t));
	
	return num;
}

INT32 heartbeat_mainloop(UINT8 *pCmd, INT32 cmdLen, g3_hb_table_t *table, UINT8 (*uplink)(UINT8 *src, UINT32 len))
{
	common_recv_parse_cmd(pCmd, cmdLen, table);
	_hb_recv(table, uplink);
	return 0;
}

void heartbeat_init()
{
	_esl_uplink_num = 0;
}


void rec_forever_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
	volatile uint8_t len  = PAYLOAD_LENGTH;
	volatile int8_t type = 0;
	static volatile int8_t i = 0;
	uint8_t* pPktDataPointer = NULL;
	st_hb *send_buf = ((HbStruct*)phb_list)->buf;				//temporary pointer
	uint8_t j;


    if (e & RF_EventRxEntryDone)
    {
        /* Get current unhandled data entry */
        currentDataEntry = RFQueue_getDataEntry();
        /* Handle the packet data, located at &currentDataEntry->data:*/
        pPktDataPointer = (uint8_t*)(&currentDataEntry->data);

        len = (pPktDataPointer[PAYLOAD_LENGTH+1] == CRC_ERR) ? 16 : PAYLOAD_LENGTH;
        type = _check_hb_data(pPktDataPointer, len);
        *(pPktDataPointer+len) = convertRSSI(pPktDataPointer[PAYLOAD_LENGTH]);

        //pinfo("%d\r\n", send_buf->num);
		//pdebughex(pPktDataPointer, len+1);

        switch(type){
			case NORMAL_DATA:
		        if((send_buf->num+1) > ONCE_TRANS_HB_NUM || send_buf->head.len+len > ONCE_TRANS_HB_BUF){
				    //pinfo("N:%x,L:%d\r\n", send_buf->num, send_buf->head.len);
		        	//pinfo("a%x,----------%d, %p 16=%d,26=%d, len=%d\r\n", phb_list, send_buf->head.len, send_buf, len16, len26, len);
		        	phb_list = List_next(phb_list);
		        	send_buf = ((HbStruct*)phb_list)->buf;
		        	//pinfo("a%x,l%d,%p\r\n", phb_list, send_buf->head.len, send_buf);
		        	RF_SemRecvPost();
		        	rf_normal_data = true;
		        	send_buf->num = 0;
		        	send_buf->head.len = 0;
		        }
		        send_buf->num++;
		        memcpy(send_buf->hb_buf+send_buf->head.len, pPktDataPointer, len+1);
		        send_buf->head.len += len+1;
				break;
			case ESL_UPLINK_DATA:
				for (j=0; j<UPLINK_BUF_NUM; j++){
					if (0 == memcmp(&esl_uplink_info[j], ((st_esl_uplink*)pPktDataPointer)->id, 5)){
						j = 0xff;
						break;
					}
				}

				if (0xff != j){
					i = i>UPLINK_BUF_NUM-1 ? 0 : i;
			        memcpy(Hb_upLinkfoo[i].buf, pPktDataPointer, len+1);
			        memcpy(&esl_uplink_info[i], ((st_esl_uplink*)pPktDataPointer)->id, 5);
			        List_put(&Hb_upLinklist, (List_Elem*)&Hb_upLinkfoo[i]);
			        i++;
			        RF_SemRecvPost();
					rf_up_data = true;
				}
				break;
//#if defined(PCIE)
            case RC_UPLINK_DATA:
                for (j=0; j<UPLINK_BUF_NUM; j++){
                    if (0 == memcmp(&esl_uplink_info[j], ((st_esl_uplink*)pPktDataPointer)->id, 5)){
                        j = 0xff;
                        break;
                    }
                }

                if (0xff != j){
                    i = i>UPLINK_BUF_NUM-1 ? 0 : i;
                    memcpy(Hb_upLinkfoo[i].buf, pPktDataPointer, len+1);
                    memcpy(&esl_uplink_info[i], ((st_esl_uplink*)pPktDataPointer)->id, 5);
                    List_put(&Hb_upLinklist, (List_Elem*)&Hb_upLinkfoo[i]);
                    i++;
                    RF_SemRecvPost();
                    rf_up_rc_data = true;
                }
                break;
//#endif
			default:
				break;
        }
        RFQueue_nextEntry();

    }else{
    	pinfo("RFerror\r\n");
    	RF_SemRecvPost();
    	rf_normal_data = false;
    	rf_up_data = false;
    }

}
