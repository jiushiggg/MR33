/*
 * tap2go.c
 *
 *  Created on: 2018年11月8日
 *      Author: ggg
 */
#include <stdint.h>
#include <ti/sysbios/knl/Clock.h>

#include "timer.h"
#include "debug.h"
#include "cc2640r2_rf.h"
#include "core.h"
#include "tap2go_event.h"
#include "crc16.h"
#include "coremem.h"
#include "tap2go.h"


typedef T2G_cmd (*eventHandle)(T2G_status* rf, core_task_t* buf);

static T2G_eventHandle T2G_init(T2G_status* rf, core_task_t* buf);
static T2G_eventHandle T2G_rxFrame1Handle(T2G_status* rf, core_task_t* buf);
static T2G_eventHandle T2G_rxFrame1Config(T2G_status* rf, core_task_t* buf);
static T2G_eventHandle T2G_rxFrame1(T2G_status* rf, core_task_t* buf);
static T2G_eventHandle T2G_rxFrame1Handle(T2G_status* rf, core_task_t* buf);
static T2G_eventHandle T2G_rxDataConfig(T2G_status* rf, core_task_t* buf);
static T2G_eventHandle T2G_rxData(T2G_status* rf, core_task_t* buf);
static T2G_eventHandle T2G_rxDataHandle(T2G_status* rf, core_task_t* buf);
static T2G_eventHandle T2G_txDataConfig(T2G_status* rf, core_task_t* buf);
static T2G_eventHandle T2G_txData(T2G_status* rf, core_task_t* buf);
static T2G_eventHandle T2G_txDataHandle(T2G_status* rf, core_task_t* buf);
static T2G_eventHandle T2G_finish(T2G_status* rf, core_task_t* buf);


static uint32_t T2G_readRemainTime(uint32_t timeout);
T2G_status t2g_status;
st_rf_para rf_para;



extern T2G_cmd T2GE_init(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_sendHandshakeConfig(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_sendComplete(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_sendHandshakeHandle(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_recvFrame1Config(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_recvFrame1Complete(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_recvFrame1Handle(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_recvDataConfig(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_recvDataComplete(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_recvDataHandle(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_errorHandle(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_recvQueryHandle(T2G_status* rf, core_task_t* buf);
extern T2G_cmd UplinkE_recvDataHandle(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_recvEndPacketHandle(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_sendResponseConfig(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_sendResponseHandle(T2G_status* rf, core_task_t* buf);
extern T2G_cmd T2GE_finish(T2G_status* rf, core_task_t* buf);

const eventHandle tap2goEventFnx[T2G_ENT_NUM]={
T2GE_finish,
T2GE_init,
T2GE_sendHandshakeConfig,
T2GE_sendComplete,
T2GE_sendHandshakeHandle,

T2GE_recvFrame1Config,
T2GE_recvFrame1Complete,
T2GE_recvFrame1Handle,

T2GE_recvDataConfig,
T2GE_recvDataComplete,
T2GE_recvDataHandle,
T2GE_recvQueryHandle,
UplinkE_recvDataHandle,
T2GE_recvEndPacketHandle,
T2GE_sendResponseConfig,
T2GE_sendResponseHandle,
T2GE_errorHandle
};



void Core_Tag2goRecMessage(core_task_t* buf)
{
    T2G_status *rf = &t2g_status;

    rf->cmd = T2G_FSM_INIT;

    while (T2G_FSM_EXIT != rf->cmd){
        switch (rf->cmd)
        {
            case T2G_FSM_INIT:
                rf->event = T2G_init(rf, buf);
                break;
            case T2G_FSM_RX_FRAME1_CONFIG:
                rf->event = T2G_rxFrame1Config(rf, buf);
                break;
            case T2G_FSM_RX_FRAME1:
                rf->event = T2G_rxFrame1(rf, buf);
                break;
            case T2G_FSM_RX_FRAME1_HANDLE:
                rf->event = T2G_rxFrame1Handle(rf, buf);
                break;

            case T2G_FSM_RX_DATA_CONFIG:
                rf->event = T2G_rxDataConfig(rf, buf);
                break;
            case T2G_FSM_RX_DATA:
                rf->event = T2G_rxData(rf, buf);
                break;
            case T2G_FSM_RX_DATA_HANDLE:
                rf->event = T2G_rxDataHandle(rf, buf);
                break;

            case T2G_FSM_TX_DATA_CONFIG:
                rf->event = T2G_txDataConfig(rf, buf);
                break;
            case T2G_FSM_TX_DATA:
                rf->event = T2G_txData(rf, buf);
                break;
            case T2G_FSM_TX_DATA_HANDLE:
                rf->event = T2G_txDataHandle(rf, buf);
                break;

            case T2G_FSM_FINISH:
                rf->event = T2G_finish(rf, buf);
                break;
            case T2G_FSM_ERROR_HANDLE:          //no break;
            default:
                rf->event = T2G_ENT_ERR_HANDLE;
                break;
        }
        rf->next_cmd = tap2goEventFnx[rf->event](rf, buf);

        T2GDEBUG(("cmd:%x,next_cmd:%x,pevent:%d,event:%d\r\n", rf->cmd, rf->next_cmd, rf->prev_event, rf->event));
        rf->cmd = rf->next_cmd;
        rf->prev_event = rf->event;
    }
}

static  T2G_eventHandle T2G_init(T2G_status* rf, core_task_t* buf)
{
    return rf->event = T2G_ENT_INIT;
}

static T2G_eventHandle T2G_rxFrame1Config(T2G_status* rf, core_task_t* buf)
{
    return rf->event = T2G_ENT_RECV_FRAME1_CONFIG;
}

static T2G_eventHandle T2G_rxFrame1(T2G_status* rf, core_task_t* buf)
{
    uint8_t ap_id[ID_LEN]={0x52, 0x56, 0x78, 0x53};

    rf_para.frame1_timeout = T2G_readRemainTime(FRAME1_TIMEOUT);
    T2GDEBUG(("Frame1 timeout remain:%dus\r\n", rf_para.frame1_timeout));
    if(recv_data(ap_id, buf->cmd_buf.buf, sizeof(st_t2g_frame1), rf_para.frame1_timeout) == 0){
        rf->event = T2G_ENT_ERR_HANDLE;
        rf->error = T2G_ERROR_RX_FRAME1_TIMEOUT;
        T2GDEBUG(("Frame1 timeout\r\n"));
    }else {
        rf->event = T2G_ENT_RECV_FRAME1_COMPLETE;
        rf->error = T2G_ERROR_NONE;
    }
    return rf->event;
}

static T2G_eventHandle T2G_rxFrame1Handle(T2G_status* rf, core_task_t* buf)
{
    uint16_t crc = 0;
    crc = CRC16_CaculateStepByStep(crc, buf->data_ptr, sizeof(st_t2g_frame1)-sizeof(crc));
    crc = CRC16_CaculateStepByStep(crc, rf_para.esl_id, 4);

    if (crc != buf->cmd_buf.t2g_frame1.crc){
        rf->error = T2G_ERROR_FRAME1_CRC;

#if defined(T2GDEBUG_LEVE)
        Debug_SetLevel(DEBUG_LEVEL_DEBUG);
#endif
        pdebughex(buf->data_ptr, sizeof(st_t2g_frame1));
        Debug_SetLevel(DEBUG_LEVEL_INFO);

        T2GDEBUG(("Frame1 CRC error:%d, len:%d\r\n", crc, sizeof(st_t2g_frame1)));
        return rf->event = T2G_ENT_ERR_HANDLE;
    }
#if defined(T2GDEBUG_LEVE)
    Debug_SetLevel(DEBUG_LEVEL_DEBUG);
#endif
    pdebughex(buf->data_ptr, sizeof(st_t2g_frame1));
    Debug_SetLevel(DEBUG_LEVEL_INFO);
    switch(buf->cmd_buf.buf[0]){
        case T2G_FRAME1_CTRL:
            rf->event = T2G_ENT_RECV_FRAME1_HANDLE;
            break;
        default:
            rf->event = T2G_ENT_ERR_HANDLE;
            rf->error = T2G_ERROR_FRAME1_CTRL;
            break;

    }

    return rf->event;
}


static T2G_eventHandle T2G_rxDataConfig(T2G_status* rf, core_task_t* buf)
{
    switch(rf->prev_event){
        case T2G_ENT_RECV_FRAME1_HANDLE:
            T2GDEBUG((" uplink recv data config\r\n"));
            rf->event = T2G_ENT_RECV_DATA_CONFIG;
            rf->error = T2G_ERROR_NONE;
            break;
        case T2G_ENT_SEND_HANDSHAKE_HANDLE:
            T2GDEBUG(("recv data config\r\n"));
            rf->event = T2G_ENT_RECV_DATA_CONFIG;
            rf->error = T2G_ERROR_NONE;
            break;
        default:
            T2GDEBUG(("rxDataConfig default\r\n"));
            rf->event = T2G_ENT_ERR_HANDLE;
            rf->error = T2G_ERROR_UNKNOWN;
            break;
    }

    return rf->event;
}

static T2G_eventHandle T2G_rxData(T2G_status* rf, core_task_t* buf)
{
    uint8_t ap_id[ID_LEN]={0x52, 0x56, 0x78, 0x53};

    rf_para.rf_timeout = T2G_readRemainTime(rf_para.timeout);
    T2GDEBUG(("Data time remain:%d\r\n", rf_para.rf_timeout));

    if(recv_data(ap_id, buf->cmd_buf.buf, rf_para.FIFO_len, rf_para.rf_timeout) == 0){
        rf->event = T2G_ENT_ERR_HANDLE;
        rf->error = T2G_ERROR_RX_TIMEOUT;
        T2GDEBUG(("rxData timeout\r\n"));
    }else {
        rf->event = T2G_ENT_RECV_DATA_COMPLETE;
        rf->error = T2G_ERROR_NONE;
    }

    return rf->event;
}

static T2G_eventHandle T2G_rxDataHandle(T2G_status* rf, core_task_t* buf)
{
    uint16_t crc = 0;
    crc = CRC16_CaculateStepByStep(crc, buf->data_ptr, rf_para.FIFO_len-sizeof(crc));
    crc = CRC16_CaculateStepByStep(crc, rf_para.esl_id, 4);

    T2GDEBUG(("rxData handle\r\n"));
    if (crc != ((uint16_t)buf->data_ptr[rf_para.FIFO_len-1]<<8 | buf->data_ptr[rf_para.FIFO_len-2])){
        rf->event = T2G_ENT_ERR_HANDLE;
        rf->error = T2G_ERROR_DATA_CRC;
        T2GDEBUG(("Data CRC error\r\n"));
    }else {
#if defined(T2GDEBUG_LEVE)
        Debug_SetLevel(DEBUG_LEVEL_DEBUG);
#endif
        pdebughex(buf->data_ptr, sizeof(st_t2g_data));
        Debug_SetLevel(DEBUG_LEVEL_INFO);
        switch(buf->cmd_buf.buf[0]){
            case UPLINK_DATA_CTRL:
                rf->event = UPLINK_ENT_RECV_DATA_HANDLE;
                rf->error = T2G_ERROR_NONE;
                break;
            case T2G_QUERY_CTRL:
                rf->event = T2G_ENT_RECV_QUERY_HANDLE;
                rf->error = T2G_ERROR_NONE;
                break;
            case T2G_COMPLETE_CTRL:
                rf->event = T2G_ENT_RECV_END_PACKET_HANDLE;
                rf->error = T2G_ERROR_NONE;
                break;
            case T2G_DATA_CTRL:
                rf->event = T2G_ENT_RECV_DATA_HANDLE;
                rf->error = T2G_ERROR_NONE;
            	break;
            default:
                rf->event = T2G_ENT_ERR_HANDLE;
                rf->error = T2G_ERROR_DATA_CTRL;
                break;
        }
    }

    return rf->event;
}



static T2G_eventHandle T2G_txDataConfig(T2G_status* rf, core_task_t* buf)
{
    switch(rf->prev_event){
        case T2G_ENT_INIT:
            rf->event = T2G_ENT_HANDSHAKE_CONFIG;
            rf->error = T2G_ERROR_NONE;
            break;
        case T2G_ENT_RECV_QUERY_HANDLE:
        	//no break;
        case T2G_ENT_RECV_DATA_HANDLE:
            rf->event = T2G_ENT_SEND_RESPONSE_CONFIG;
            rf->error = T2G_ERROR_NONE;
            break;
        default:
            T2GDEBUG(("txDataConfig default\r\n"));
            rf->event = T2G_ENT_ERR_HANDLE;
            rf->error = T2G_ERROR_UNKNOWN;
            break;
    }

    return rf->event;
}
extern void BSP_Delay1MS(INT32 delayMs);
static T2G_eventHandle T2G_txData(T2G_status* rf, core_task_t* buf)
{
	uint8_t len=0;
	int8_t i=0;	
    switch(rf->prev_event){
        case T2G_ENT_HANDSHAKE_CONFIG:
            T2GDEBUG(("txData handshake:%d\r\n", sizeof(st_t2g_down_rsp)));
            for (i=0; i<2; i++){
            	send_data(rf_para.esl_id, buf->ack_buf.buf, sizeof(st_t2g_down_rsp), 2000);
            }
            rf->event = T2G_ENT_SEND_COMPLETE;
            rf->error = T2G_ERROR_NONE;
            break;
        case T2G_ENT_SEND_RESPONSE_CONFIG:
            T2GDEBUG(("txData response\r\n"));
            BSP_Delay1MS(3);			//for Telink
            if (UPLINK_GEOLOCATION_HB == rf_para.req_type){
            	i   = 2;				//for Telink send twice
            	len = rf_para.FIFO_len; 
            }else if(UPLINK_T2G_MODE == rf_para.req_type){
            	i   = 1;
            	len = T2G_ACK_LEN;
            }else{
            	rf->event = T2G_ENT_ERR_HANDLE;
            	rf->error = T2G_ERROR_UNKNOWN;
            	break;
            }
            for (; i>0; i--){
            	send_data(rf_para.esl_id, buf->ack_buf.buf, len, 10000);
            }
            rf->event = T2G_ENT_SEND_COMPLETE;
            rf->error = T2G_ERROR_NONE;

#if defined(T2GDEBUG_LEVE)
            Debug_SetLevel(DEBUG_LEVEL_DEBUG);
#endif
            pdebughex(buf->ack_buf.buf, len);
            Debug_SetLevel(DEBUG_LEVEL_INFO);
            break;
        default:
            T2GDEBUG(("txData default\r\n"));

            rf->event = T2G_ENT_ERR_HANDLE;
            rf->error = T2G_ERROR_UNKNOWN;
            break;

    }

    return rf->event;
}

static T2G_eventHandle T2G_txDataHandle(T2G_status* rf, core_task_t* buf)
{
    switch(buf->ack_buf.buf[0]){
        case T2G_HANDSHAKE_CTRL:
            rf->event = T2G_ENT_SEND_HANDSHAKE_HANDLE;
            rf->error = T2G_ERROR_NONE;
            break;
        case T2G_RESPONSE_CTRL:
            rf->event = T2G_ENT_SEND_RESPONSE_HANDLE;
            rf->error = T2G_ERROR_NONE;
            break;
        default:
            T2GDEBUG(("txDataHandle default\r\n"));

            rf->event = T2G_ENT_ERR_HANDLE;
            rf->error = T2G_ERROR_UNKNOWN;
            break;
    }

    return rf->event;
}

static T2G_eventHandle T2G_finish(T2G_status* rf, core_task_t* buf)
{
    //todo:结束处理
    T2GDEBUG(("T2G_finish\r\n"));
    return T2G_ENT_FINISH;
}

//timeout: 单位ms.超时时间
//return: 单位:us.remain times
static uint32_t T2G_readRemainTime(uint32_t timeout)
{
    uint32_t ticks = 0;
    timeout *= 1000/Clock_tickPeriod;   //the ticks of timeout
    ticks = TIM_GetTicks();             //已经运行的ticks
    T2GDEBUG(("timeout%d,ticks%d\r\n", timeout, ticks));
    ticks = ticks>timeout ? 0 : timeout - ticks ;
    return ticks*Clock_tickPeriod;
}
































