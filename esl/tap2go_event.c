/*
 * tap2go_event.c
 *
 *  Created on: 2018年11月21日
 *      Author: ggg
 */

#include <stdint.h>
#include <string.h>
#include <ti/sysbios/knl/Clock.h>
#include <stddef.h>

#include "updata.h"
#include "crc16.h"
#include "debug.h"
#include "core.h"
#include "corefunc.h"
#include "cc2640r2_rf.h"
#include "timer.h"
#include "tap2go.h"
#include "coremem.h"
#include "tap2go_event.h"

#define PACKET_MASK         0X3FFF
#define LAST_PACKET_MASK	(0XC000)
#define CRC_LEN             2
#define TAP2GO_UPLINK_SIZE	4096


#pragma pack(1)
typedef struct data_struct{
    uint8_t pre_sid;            //
    uint8_t pre_apid;           //
    uint8_t slot;               //
//    uint16_t lost_packet;       //丢包数
    uint16_t recv_num;          //收到包数
    uint16_t total_num;         //总包数
    uint16_t first_lose_pkg;
    uint16_t rssi;
    uint16_t packet_cnt;
}data_struct;

typedef struct st_info_head{
    uint16_t cmd;
    uint32_t cmd_len;
    uint8_t els_id[4];
    uint8_t ap_channel;
    uint8_t rssi;
    uint16_t packet_num;
}st_info_head;
#pragma pack()

T2G_cmd T2GE_init(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_sendHandshakeConfig(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_sendComplete(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_sendHandshakeHandle(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_recvFrame1Config(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_recvFrame1Complete(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_recvFrame1Handle(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_recvDataConfig(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_recvDataComplete(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_recvDataHandle(T2G_status* rf, core_task_t* buf);
T2G_cmd UplinkE_recvDataHandle(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_errorHandle(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_recvQueryHandle(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_recvEndPacketHandle(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_sendResponseConfig(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_sendResponseHandle(T2G_status* rf, core_task_t* buf);
T2G_cmd T2GE_finish(T2G_status* rf, core_task_t* buf);


//static uint32_t T2G_readRemainTime(uint32_t timeout);
static uint16_t T2G_powerMap(uint8_t rate);
static uint16_t T2G_rateMap(uint8_t rate);

static uint8_t packet_bitmap(uint16_t packet_num);
static uint16_t lose_pkg_statistics(uint16_t pkg_all,uint8_t *buf, uint16_t buf_len);
static void T2G_checkSID(uint8_t cur_sid);

static uint8_t timer;

static data_struct packet;
static uint8_t* p_uplink_info=NULL;

static uint8_t* pkg_bit_map;
T2G_cmd T2GE_init(T2G_status* rf, core_task_t* buf)
{
    buf->data_ptr = buf->cmd_buf.buf;
    buf->ack_ptr = buf->ack_buf.buf;

    rf_para.default_rate    = buf->cmd_buf.t2g.default_rate;
    rf_para.default_power   = buf->cmd_buf.t2g.default_power;
    rf_para.default_ch      = buf->cmd_buf.t2g.default_ch;
    memcpy(rf_para.esl_id, buf->cmd_buf.t2g.id, ID_LEN);
    rf_para.ap_ch              = buf->cmd_buf.t2g.ap_ch;
    rf_para.ap_id           = buf->cmd_buf.t2g.ap_id;
    rf_para.recv_info_ch    = buf->cmd_buf.t2g.recv_info_ch;
    rf_para.recv_info_rate  = T2G_rateMap(buf->cmd_buf.t2g.req.rate_bitmap);
    rf_para.recv_info_power = T2G_powerMap(buf->cmd_buf.t2g.req.power_bitmap);
    rf_para.FIFO_len        = buf->cmd_buf.t2g.req.RF_FIFO;
    rf_para.timeout         = buf->cmd_buf.t2g.timeout;
    rf_para.rf_timeout      = rf_para.timeout*1000;
    rf_para.data_len        = buf->cmd_buf.t2g.req.data_len;
    rf_para.req_type		= buf->cmd_buf.t2g.req_type;

    pkg_bit_map=Core_Malloc(G_PKG_BIT_MAP_LEN);
    memset((uint8_t*)&packet, 0, sizeof(packet));
//    packet.lost_packet      = 0xff;

    if (rf_para.data_len < TAP2GO_UPLINK_SIZE){
        p_uplink_info = Core_Malloc(TAP2GO_UPLINK_SIZE);
    }else {
        //todo:申请flash空间
    }

    T2GDEBUG(("handshake parameters rate:%d,power:%d,ch:%d\r\n", rf_para.default_rate, rf_para.default_power, rf_para.default_ch));
    T2GDEBUG(("receive uplink info parameters rate:%d,power:%d,ch:%d,FIFO:%d\r\n",
            rf_para.recv_info_rate, rf_para.recv_info_power, rf_para.recv_info_ch,rf_para.FIFO_len));
    T2GDEBUG(("eslid:%x%x%x%x\r\n",rf_para.esl_id[0],rf_para.esl_id[1],rf_para.esl_id[2],rf_para.esl_id[3]));
    T2GDEBUG(("update parameters ch:%d,ap_id:%d\r\n",rf_para.ap_ch,rf_para.ap_id));
    T2GDEBUG(("update parameters timeout:%d,data len:%d\r\n",rf_para.timeout,rf_para.data_len ));

    return T2G_FSM_TX_DATA_CONFIG;
}

T2G_cmd T2GE_sendHandshakeConfig(T2G_status* rf, core_task_t* buf)
{
    set_power_rate(rf_para.default_power, rf_para.default_rate);
    set_frequence(rf_para.default_ch);

    buf->ack_buf.t2g_rsp.ctrl           = T2G_HANDSHAKE_CTRL;
    memcpy(buf->ack_buf.t2g_rsp.id, rf_para.esl_id, ID_LEN);
    buf->ack_buf.t2g_rsp.ap_id          = rf_para.ap_id;			//buf->cmd_buf.t2g.ap_id;
    buf->ack_buf.t2g_rsp.esl_uplink_ch  = rf_para.recv_info_ch; 	//buf->cmd_buf.t2g.recv_info_ch;
    buf->ack_buf.t2g_rsp.session_id     = buf->cmd_buf.t2g.session_id;
    buf->ack_buf.t2g_rsp.ap_ch          = rf_para.ap_ch;
    buf->ack_buf.t2g_rsp.status         = buf->cmd_buf.t2g.status;
    buf->ack_buf.t2g_rsp.rate_bitmap    = buf->cmd_buf.t2g.req.rate_bitmap;
    buf->ack_buf.t2g_rsp.power_bitmap   = buf->cmd_buf.t2g.req.power_bitmap;
    buf->ack_buf.t2g_rsp.RF_FIFO        = rf_para.FIFO_len;			//buf->cmd_buf.t2g.req.RF_FIFO;
    memset(buf->ack_buf.t2g_rsp.reserve, 0, sizeof(buf->ack_buf.t2g_rsp.reserve));

    buf->ack_buf.t2g_rsp.crc = CRC16_CaculateStepByStep(0, buf->ack_buf.buf,
                                                        sizeof(st_t2g_down_rsp)-sizeof(buf->ack_buf.t2g_rsp.crc));
    buf->ack_buf.t2g_rsp.crc = CRC16_CaculateStepByStep(buf->ack_buf.t2g_rsp.crc, rf_para.esl_id, ID_LEN);

    return T2G_FSM_TX_DATA;
}

T2G_cmd T2GE_sendComplete(T2G_status* rf, core_task_t* buf)
{
    return T2G_FSM_TX_DATA_HANDLE;
}

T2G_cmd T2GE_sendHandshakeHandle(T2G_status* rf, core_task_t* buf)
{
	T2G_cmd ret;
	switch(rf_para.req_type){
		case UPLINK_T2G_MODE:
			ret = T2G_FSM_RX_DATA_CONFIG;
			break;
		case UPLINK_GEOLOCATION_HB:
			//no break;
		default:
			ret = T2G_FSM_RX_FRAME1_CONFIG;
			break;
	}
    return ret;
}

T2G_cmd T2GE_sendResponseHandle(T2G_status* rf, core_task_t* buf)
{
	T2G_cmd ret;
	switch(rf_para.req_type){
		case UPLINK_T2G_MODE:
			ret = T2G_FSM_FINISH;
			break;
		case UPLINK_GEOLOCATION_HB:
			//no break;
		default:
			ret = T2G_FSM_RX_DATA;
			break;
	}
    return ret;
    //return T2G_FSM_RX_DATA;
}

T2G_cmd T2GE_recvFrame1Config(T2G_status* rf, core_task_t* buf)
{
    T2G_cmd tmp = T2G_FSM_CMD_UNKNOWN;

    set_power_rate(RF_DEFAULT_POWER, 100);
    set_frequence(rf_para.default_ch);

    if((timer=TIM_Open(FRAME1_TIMEOUT, 1, TIMER_DOWN_CNT, TIMER_ONCE)) == TIMER_UNKNOW){
        tmp = T2G_FSM_ERROR_HANDLE;
    }else{
        tmp = T2G_FSM_RX_FRAME1;
    }
    return tmp;
}

T2G_cmd T2GE_recvFrame1Complete(T2G_status* rf, core_task_t* buf)
{
    return T2G_FSM_RX_FRAME1_HANDLE;
}

T2G_cmd T2GE_recvFrame1Handle(T2G_status* rf, core_task_t* buf)
{
    T2G_cmd cmd = T2G_FSM_RX_FRAME1;

    switch(buf->cmd_buf.buf[0]&CTRL_MASK){
        case T2G_FRAME1_CTRL:
            if (buf->cmd_buf.t2g_frame1.req.ap_id != rf_para.ap_id){
            	pinfo("error apid%d, %d\r\n", buf->cmd_buf.t2g_frame1.req.ap_id, rf_para.ap_id);
                break;
            }
            if (0 != memcmp(buf->cmd_buf.t2g_frame1.id, rf_para.esl_id, sizeof(rf_para.esl_id))){
                pinfo("errid:%x%x%x%x\r\n", rf_para.esl_id[0],rf_para.esl_id[1],rf_para.esl_id[2],rf_para.esl_id[3]);
                break;
            }
            if (UPLINK_T2G_VERIFY != buf->cmd_buf.t2g_frame1.req_type){
                pinfo("errtype:%x\r\n",buf->cmd_buf.t2g_frame1.req_type);
                break;
            }
            if (buf->cmd_buf.t2g_frame1.session_id != buf->ack_buf.t2g_rsp.session_id){
                pinfo("err session id:%d, %d\r\n",buf->cmd_buf.t2g_frame1.session_id, buf->ack_buf.t2g_rsp.session_id);
                break;
            }
            rf_para.recv_info_rate  = T2G_rateMap(buf->cmd_buf.t2g_frame1.req.rate_bitmap);
            rf_para.FIFO_len        = buf->cmd_buf.t2g.req.RF_FIFO;
            rf_para.data_len        = buf->cmd_buf.t2g.req.data_len;
            TIM_Close(timer);
            cmd = T2G_FSM_RX_DATA_CONFIG;
            break;
        default:
            break;
    }

    return cmd;
}

T2G_cmd T2GE_recvDataConfig(T2G_status* rf, core_task_t* buf)
{
    T2G_cmd tmp = T2G_FSM_CMD_UNKNOWN;

    set_power_rate(RF_DEFAULT_POWER, rf_para.recv_info_rate);
    set_frequence(rf_para.recv_info_ch);

//    memset(&packet, 0, sizeof(packet));
    if((timer=TIM_Open(rf_para.timeout, 1, TIMER_DOWN_CNT, TIMER_ONCE)) == TIMER_UNKNOW){
        tmp = T2G_FSM_ERROR_HANDLE;
    }else{
        tmp = T2G_FSM_RX_DATA;
    }
    return tmp;
}

T2G_cmd T2GE_recvDataComplete(T2G_status* rf, core_task_t* buf)
{
	packet.rssi += get_recPkgRSSI();
	packet.packet_cnt++;
    return T2G_FSM_RX_DATA_HANDLE;
}

T2G_cmd UplinkE_recvDataHandle(T2G_status* rf, core_task_t* buf)
{
    uint16_t info_len = rf_para.FIFO_len - CRC_LEN;
    T2GDEBUG(("recv info len:%d\r\n", info_len));

    if (0 == buf->cmd_buf.uplink_data.sid){
        T2GDEBUG(("sid error:%d\r\n", buf->cmd_buf.t2g_data.sid));
        return T2G_FSM_RX_DATA;
    }
    if (0 == buf->cmd_buf.uplink_data.package_num&PACKET_MASK){
        T2GDEBUG(("packet num error:%d\r\n", buf->cmd_buf.uplink_data.package_num));
        return T2G_FSM_RX_DATA;
    }

    T2G_checkSID(buf->cmd_buf.uplink_data.sid);


    if (0 == packet_bitmap(buf->cmd_buf.uplink_data.package_num&PACKET_MASK)){
        return T2G_FSM_RX_DATA;
    }
    packet.recv_num++;
    memcpy(p_uplink_info+sizeof(st_info_head)+2+( packet.recv_num-1)*info_len,
              buf->cmd_buf.buf, info_len);


    return T2G_FSM_RX_DATA;
}

T2G_cmd T2GE_recvDataHandle(T2G_status* rf, core_task_t* buf)
{
    uint16_t info_len = rf_para.FIFO_len - CRC_LEN;
    T2GDEBUG(("recv info len:%d\r\n", info_len));

    if (0 == buf->cmd_buf.t2g_data.sid){
        T2GDEBUG(("sid error:%d\r\n", buf->cmd_buf.t2g_data.sid));
        return T2G_FSM_RX_DATA;
    }
    if (0 == buf->cmd_buf.t2g_data.package_num&PACKET_MASK){
        T2GDEBUG(("packet num error:%d\r\n", buf->cmd_buf.t2g_data.package_num));
        return T2G_FSM_RX_DATA;
    }

    T2G_checkSID(buf->cmd_buf.t2g_data.sid);


    if (0 == packet_bitmap(buf->cmd_buf.t2g_data.package_num&PACKET_MASK)){
        return T2G_FSM_RX_DATA;
    }
    packet.recv_num++;
    memcpy(p_uplink_info+sizeof(st_info_head)+((buf->cmd_buf.t2g_data.package_num&PACKET_MASK)-1)*info_len,
           buf->cmd_buf.buf, info_len);

    if (buf->cmd_buf.t2g_data.rx_ack_flg == TRUE){
    	packet.total_num = (buf->cmd_buf.t2g_data.package_num&LAST_PACKET_MASK) ? \
    			           buf->cmd_buf.t2g_data.package_num&PACKET_MASK : PACKET_MASK;
    	return T2G_FSM_TX_DATA_CONFIG;
    }else{
    	return T2G_FSM_RX_DATA;
    }

}


T2G_cmd T2GE_recvQueryHandle(T2G_status* rf, core_task_t* buf)
{
//    uint8_t buf_len = 0;
    T2GDEBUG(("recv query\r\n"));
    if (0 == buf->cmd_buf.t2g_query.sid){
        return T2G_FSM_RX_DATA;
    }
    T2G_checkSID(buf->cmd_buf.t2g_query.sid);

//    buf_len = rf_para.FIFO_len-offsetof(st_t2g_ack, lost_packet)-CRC_LEN;
//    T2GDEBUG(("recv_num=%d, total_send=%d, buf_len=%d\r\n", packet.recv_num, buf->cmd_buf.t2g_query.total_send, buf_len));
//
//    if (sizeof(buf->ack_buf.t2g_ack.lost_packet)<buf_len){
//    	buf_len = sizeof(buf->ack_buf.t2g_ack.lost_packet);
//    }
//    memset(buf->ack_buf.t2g_ack.lost_packet, 0, buf_len);
//
//    if (packet.recv_num != buf->cmd_buf.t2g_query.total_send){
//        packet.lost_packet = lose_pkg_statistics(buf->cmd_buf.t2g_query.total_send, buf->ack_buf.t2g_ack.lost_packet,buf_len);
//        T2GDEBUG(("error lost packet:%d\r\n", packet.lost_packet));
//    } else {
//        packet.lost_packet = 0;
//    }
    packet.total_num = buf->cmd_buf.t2g_query.total_send;
    packet.slot = buf->cmd_buf.t2g_query.slot;
    return T2G_FSM_TX_DATA_CONFIG;;
}

T2G_cmd T2GE_sendResponseConfig(T2G_status* rf, core_task_t* buf)
{
	uint8_t buf_len = 0, data_len = 0;
	uint16_t crc = 0;

    set_power_rate(RF_DEFAULT_POWER, rf_para.recv_info_rate);
    set_frequence(rf_para.recv_info_ch);

    buf->ack_buf.t2g_ack.ctrl       = T2G_RESPONSE_CTRL;
    buf->ack_buf.t2g_ack.sid        = packet.pre_sid;
    buf->ack_buf.t2g_ack.slot       = ++packet.slot;
//    buf->ack_buf.t2g_ack.loss_num   = packet.lost_packet;
    buf->ack_buf.t2g_ack.reserve    = 0;

    if (UPLINK_GEOLOCATION_HB==rf_para.req_type){
    	buf_len  = sizeof(st_t2g_ack) - offsetof(st_t2g_ack, lost_packet)-CRC_LEN;
    	data_len = rf_para.FIFO_len - sizeof(buf->ack_buf.t2g_ack.crc);
    }else {
    	buf_len  = T2G_ACK_LEN - offsetof(st_t2g_ack, lost_packet)-CRC_LEN;
    	data_len = T2G_ACK_LEN - sizeof(buf->ack_buf.t2g_ack.crc);
    }

    T2GDEBUG(("recv_num=%d, total_send=%d, buf_len=%d\r\n", packet.recv_num, packet.total_num, buf_len));

    if (sizeof(buf->ack_buf.t2g_ack.lost_packet)<buf_len){
    	buf_len = sizeof(buf->ack_buf.t2g_ack.lost_packet);
    }
    memset(buf->ack_buf.t2g_ack.lost_packet, 0, buf_len);

    if (packet.recv_num != packet.total_num){
    	buf->ack_buf.t2g_ack.loss_num = lose_pkg_statistics(packet.total_num, buf->ack_buf.t2g_ack.lost_packet,buf_len);
        T2GDEBUG(("error lost packet:%d\r\n", buf->ack_buf.t2g_ack.loss_num));
    }else {
    	buf->ack_buf.t2g_ack.loss_num = 0;
    }

	crc = CRC16_CaculateStepByStep(0, buf->ack_buf.buf, data_len);
	crc = CRC16_CaculateStepByStep(crc, rf_para.esl_id, ID_LEN);
        buf->ack_buf.buf[data_len+1] = (crc>>8)&0xff;
        buf->ack_buf.buf[data_len] = crc&0xff;

    return T2G_FSM_TX_DATA;
}

T2G_cmd T2GE_recvEndPacketHandle(T2G_status* rf, core_task_t* buf)
{
    T2GDEBUG(("recv end\r\n"));

    if (buf->cmd_buf.t2g_complete.sid != packet.pre_sid){
        T2GDEBUG(("end sid error:%d, self sid:%d\r\n", buf->cmd_buf.t2g_complete.sid, packet.pre_sid));
        return T2G_FSM_RX_DATA;
    }

    if (buf->cmd_buf.t2g_complete.slot != packet.slot+1){
        T2GDEBUG(("end solt error:%d\r\n", buf->cmd_buf.t2g_complete.slot, packet.slot+1));
        return T2G_FSM_RX_DATA;
    }

    TIM_Close(timer);

    return T2G_FSM_FINISH;
}

T2G_cmd T2GE_errorHandle(T2G_status* rf, core_task_t* buf)
{
    T2G_cmd cmd = T2G_FSM_CMD_UNKNOWN;

    switch(rf->error){
        case T2G_ERROR_DATA_CRC:
        case T2G_ERROR_DATA_CTRL:
        case T2G_ERROR_DATA:
//            rf_para.rf_timeout = T2G_readRemainTime(rf_para.timeout);
//            T2GDEBUG(("dataTimeout remain:%d/r/n", rf_para.rf_timeout));
            T2GDEBUG(("data error ctrl/r/n"));
            cmd = T2G_FSM_RX_DATA;
            break;
        case T2G_ERROR_FRAME1_CRC:
        case T2G_ERROR_FRAME1_CTRL:
        case T2G_ERROR_FRAME1_DATA:
//            rf_para.frame1_timeout = T2G_readRemainTime(FRAME1_TIMEOUT);
//            T2GDEBUG(("frame1Timeout remain:%d/r/n", rf_para.rf_timeout));
            T2GDEBUG(("frame1 error ctrl/r/n"));
            cmd = T2G_FSM_RX_FRAME1;
            break;
        case T2G_ERROR_TX_TIMEOUT:
            cmd = T2G_FSM_FINISH;
            break;
        case T2G_ERROR_RX_TIMEOUT:
            T2GDEBUG(("rxTimeoutError.Close timer\r\n"));
            TIM_Close(timer);
            if (UPLINK_GEOLOCATION_HB==rf_para.req_type){
                packet.total_num=packet.recv_num;
            }
            cmd = T2G_FSM_FINISH;
            break;
        case T2G_ERROR_RX_FRAME1_TIMEOUT:
            T2GDEBUG(("frame1Timeout.Close timer\r\n"));
            TIM_Close(timer);
            cmd = T2G_FSM_FINISH;
            break;
        case T2G_ERROR_CAL:
            cmd = T2G_FSM_FINISH;
            break;
        case T2G_ERROR_UNKNOWN:
        default:
            cmd = T2G_FSM_FINISH;
            TIM_Close(timer);
            break;
    }

    return cmd;
}


T2G_cmd T2GE_finish(T2G_status* rf, core_task_t* buf)
{
    st_info_head *tmp_head = (st_info_head*)p_uplink_info;
    T2GDEBUG(("T2GE_finish:%d,%d\r\n", packet.recv_num, packet.total_num));
    TIM_Close(timer);
    if (packet.recv_num==packet.total_num && packet.recv_num!=0){
        if (rf_para.data_len < TAP2GO_UPLINK_SIZE){       //Data in RAM
            buf->flash_ack_len = 0;
            tmp_head->cmd_len = packet.recv_num*(rf_para.FIFO_len-CRC_LEN)+sizeof(st_info_head)-sizeof(tmp_head->cmd)-sizeof(tmp_head->cmd_len);
			if (UPLINK_GEOLOCATION_HB==rf_para.req_type){
				tmp_head->cmd = 0x1100;
				tmp_head->cmd_len += sizeof(packet.recv_num);
				memcpy(p_uplink_info+sizeof(st_info_head), &packet.recv_num, sizeof(packet.recv_num));
			}else{
				tmp_head->cmd = 0x10f0;
			}			
            //pinfo("tmp_head->cmd_len:%d,%d,%d\r\n", tmp_head->cmd_len,packet.recv_num,rf_para.FIFO_len);
            memcpy(tmp_head->els_id, rf_para.esl_id, sizeof(rf_para.esl_id));
            tmp_head->packet_num = packet.recv_num;
            tmp_head->rssi = packet.rssi/packet.packet_cnt;
            tmp_head->ap_channel = rf_para.ap_ch;
            pdebug("rf_para.ap_ch:%d\r\n",rf_para.ap_ch);
            pinfo("cmd_len:%d,%d\r\n", tmp_head->cmd_len,packet.recv_num);
//#define TMP_DEBUG			//because imx6ull doesn't modify
#if defined(TMP_DEBUG)			
            memcpy(p_uplink_info+sizeof(st_info_head)+5,
                   p_uplink_info+sizeof(st_info_head)+7, 119);
#endif				   
            Core_SendData(p_uplink_info, tmp_head->cmd_len+sizeof(tmp_head->cmd)+sizeof(tmp_head->cmd_len));
        }else{              //Data in external flash
            buf->flash_ack_len = 1;
            buf->flash_ack_addr = 1;
            buf->ack_ptr = NULL;
            buf->ack_len = 0;
            Core_SendDataFromFlash(buf->flash_ack_addr, buf->flash_ack_len);
        }
    }else{
        buf->ack_ptr = NULL;
        buf->ack_len = 0;
        buf->flash_ack_len = 0;
        pinfo("local_task.ack_len=%d\r\n", buf->ack_len);
        Core_SendAck(CORE_CMD_ACK, 0, NULL);
    }

    if (rf_para.data_len < TAP2GO_UPLINK_SIZE){
    	Core_Free(p_uplink_info);
    	//Core_Free(pkg_bit_map);
    }else {
    	//Core_Free(pkg_bit_map);
    }

    Core_Free(pkg_bit_map);
    return T2G_FSM_EXIT;
}

//return: 0:have been received packet. 1: new packet
static uint8_t packet_bitmap(uint16_t packet_num)
{
#ifdef PGK_BIT_FLASH_OPEN
    uint16_t packet_byte = 0,offset = 0;
    uint8_t packet_bit = 0,des = 0xff,tp=0;


    packet_num -=1;
    packet_byte = packet_num / 8;
    packet_bit  = (uint8_t)(packet_num % 8);
    offset += packet_byte;

    f_read(file_id,(WORD)offset,&des,sizeof(des));
    if( (des & (0x01<<packet_bit)) == 0)
        return 0;
    tp = (0x01<<packet_bit);
    tp = ~tp;
    des &= tp;
    f_write_direct(file_id,offset,&des,sizeof(des));

    return 1;
#else
    uint16_t packet_byte = 0,offset = 0;
    uint8_t packet_bit = 0;

    packet_num -=1;
    packet_byte = packet_num / 8;
    packet_bit  = (uint8_t)(packet_num % 8);
    offset += packet_byte;
    if(offset >= G_PKG_BIT_MAP_LEN)
    {
        return 0;
    }
    if((pkg_bit_map[offset] & (0x01<<packet_bit)) == 0)
        return 0;

    pkg_bit_map[offset] &= ~(0x01<<packet_bit);
    return 1;
#endif
}

static uint16_t lose_pkg_statistics(uint16_t pkg_all,uint8_t *buf, uint16_t buf_len)
{
#ifdef PGK_BIT_FLASH_OPEN
    uint16_t j,all_pkg_byte,lose, n;
    uint8_t temp[64],i, read_byte,k, len = 0;

    all_pkg_byte = ( (pkg_all/8) +1);
    for( j = ((first_lose_pkg)/8>0)?(first_lose_pkg)/8 -1:0; j < all_pkg_byte ; j+= read_byte)
    {

        if(all_pkg_byte - j >= 64)
            read_byte = 64;
        else
            read_byte = all_pkg_byte - j;

        f_read(readid,j,temp,read_byte);

        for(i = 0 ; i < read_byte ; i++)
        {
            if(temp[i] == 0x00)
                continue;

            for(k = 0; k < 8; k++)
            {
                n = (j + i) * 8 + k;
                if(n == pkg_all)
                    goto End;
                if((temp[i] & ( 0x01<<k)) == 0)
                    continue;
                if(len == 20)
                    goto End;

                lose =  n +1;
                buf[len++] = lose & 0xff;
                buf[len++] = (lose >> 8) & 0xff;
            }
        }

    }

    End:
    if (len == 0) {
        first_lose_pkg = pkg_all;
    } else {
        first_lose_pkg = buf[0] + buf[1]*256;
    }

    return len /2;
#else
    uint16_t i,j,all_pkg_byte,lose, n;
    uint8_t  k, len = 0;

    all_pkg_byte = ( (pkg_all/8) +1);
    j = ((packet.first_lose_pkg)/8>0)?(packet.first_lose_pkg)/8 - 1:0;//第一个丢包的起始字节

    for(i = j ; i < all_pkg_byte ; i++)
    {
        if(pkg_bit_map[i] == 0x00)
            continue;

        for(k = 0; k < 8; k++)
        {
            n = i * 8 + k;
            if(n == pkg_all)
                goto End;
            if((pkg_bit_map[i] & ( 0x01<<k)) == 0)
                continue;
            if(len == buf_len)
                goto End;

            lose =  n +1;
            buf[len++] = lose & 0xff;
            buf[len++] = (lose >> 8) & 0xff;
        }
    }
    End:
    if (len == 0) {
        packet.first_lose_pkg = pkg_all;
    } else {
        packet.first_lose_pkg = buf[0] + buf[1]*256;
    }

    return len /2;
#endif
}


static void T2G_checkSID(uint8_t cur_sid)
{
    if (cur_sid != packet.pre_sid){       //
        memset(&packet, 0, sizeof(packet));
        memset(pkg_bit_map, 0xff, sizeof(pkg_bit_map));
        packet.pre_sid = cur_sid;
//        packet.lost_packet = 0xff;

    }
}

//esl bit map
//Bit0:100k，bit1:125k，bit2:500k，bit3:1M，bit6:1.5M，bit7:2M
//bit7:-29db bit6:-28db bit5:5db bit4:4db bit3:3db bit2:2db bit1:1db bit0:0db

static uint16_t T2G_rateMap(uint8_t rate)
{
    uint16_t ret = 500;
    switch(rate){
        case RATE_100K:
            ret = 100;
            break;
        case RATE_125K:
            ret = 125;
            break;
        case RATE_500K:
            ret = 500;
            break;
        case RATE_1M:
            ret = 10;
            break;
        case RATE_1D5M:
            ret = 15;
            break;
        case RATE_2M:
            ret = 20;
            break;
        default:
            ret = 500;
            break;
    }

    return ret;
}

static uint16_t T2G_powerMap(uint8_t rate)
{
    uint16_t ret = 500;
    switch(rate){
        case POWER_0:
            ret = 3;
            break;
        case POWER_6:
            ret = 2;
            break;
        case POWER_10:
            ret = 1;
            break;
        case POWER_13:
            ret = 0;
            break;
        default:
            ret = 2;
            break;
    }
    return ret;
}
