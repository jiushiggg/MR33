#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Power.h>

#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/drivers/Power/PowerCC26XX.h>
#include <xdc/runtime/Error.h>

/* Board Header file */
#include "Board.h"
#include "datatype.h"
#include "sys_cfg.h"
#include "extern_flash.h"
#include "core.h"
#include "timer.h"
#include "debug.h"
#include "flash.h"
#include "esl.h"
#include "corefunc.h"
#include "coremem.h"
#include "corehandlecmd.h"
#include "xmodem.h"
#include "bsp.h"
#include "rftest.h"
#include "rcuplink.h"
#include "heartbeat.h"
#include "updata.h"
#include "thread.h"
#include "cc2640r2_rf.h"
#include "event.h"
#include "protocol.h"
#include "SPI_private.h"
#include "storage.h"
#include "watchdog.h"
#include "appSPI.h"

extern void Core_Tag2goRecMessage(core_task_t* buf);

#pragma location=(CORE_TASK_ADDR)
core_task_t local_task;
volatile uint8_t core_idel_flag = 0;
void (*tim_soft_callback)(void);
st_flash_data tmp_data;

//����ack_data�ĵ�ַ������local_task.ack_buf�ĵ�ַ�г�ͻ��
uint8_t Core_SendAck(uint16_t ack_cmd, uint32_t ack_len, uint8_t *ack_data)
{
    INT32 tx_ack_ret = 0;
    UINT8 ret = 0;
    INT32 len = ack_len+2+4;
    sn_t x;

    if(len > sizeof(un_ack_buf))
    {
        perr("Core_SendAck() cmd len too big.\r\n");
        goto done;
    }
    memcpy((void *)&local_task.ack, (void *)&ack_cmd, sizeof(ack_cmd));
    memcpy((void *)&local_task.ack_len, (void *)&ack_len, sizeof(ack_len));

    if((ack_len!=0) && (ack_data!=NULL) && ack_data!=(uint8_t*)&local_task.ack_buf)
    {
        memcpy((uint8_t*)&local_task.ack_buf, ack_data, ack_len);
    }
    memset(&x, 0, sizeof(sn_t));

    tx_ack_ret = protocol_send(&x, (uint8_t*)&local_task.ack, len, EVENT_WAIT_US(50000));
    if(tx_ack_ret == len)
    {
        pdebug("Core_SendAck 0x%04X\r\n", ack_cmd);
        ret = 1;
    }
    else
    {
        perr("Core_SendAck 0x%04X return %d\r\n", ack_cmd, tx_ack_ret);
        ret = 0;
    }

done:
    return ret;
}



uint8_t Core_GetQuitStatus(void)
{
	return core_idel_flag;
}
extern bool SPI_appRecv(void *buffer, uint16_t size);
void Core_ResetQuitStatus()
{
	core_idel_flag = 0;
#if defined(AP_3)||defined(PCIE_SPI)
	SPI_appRecv(NULL, SPIPRIVATE_LEN_ALL);
#endif
}

void Core_Init(void)
{
	heartbeat_init();
	
	core_idel_flag = 0;
	
	memset(&local_task, 0 , sizeof(core_task_t));
}

void Core_TxHandler(void)
{
	/* tx ack */	
	if(Core_SendAck(local_task.ack, local_task.ack_len, local_task.ack_ptr) == 0)
	{
		perr("Core_TxHandler tx cmd 0x%04X\r\n", local_task.ack);
	}
	else
	{
		pinfo("Core_TxHandler tx cmd 0x%04X success!\r\n", local_task.ack);
	}
}

void Core_RxHandler(void)
{
	memcpy((void *)&local_task.cmd, local_task.data_ptr, 2);
	memcpy((void *)&local_task.cmd_len, local_task.data_ptr+2, 4);
//	pdebug("Core_RxHandler cmd=0x%04X,len=%d\r\n", local_task.cmd, local_task.cmd_len);
	pinfo("cmd=0x%04X,len=%d\r\n", local_task.cmd, local_task.cmd_len);
//	pdebughex(local_task.data_ptr, local_task.data_len);
	if(local_task.cmd_len > sizeof(local_task.cmd_buf.buf))
	{
		perr("Core_RxHandler() cmd len too big!\r\n");
		goto done;
	}	
	memcpy(local_task.cmd_buf.buf, local_task.data_ptr+6, local_task.cmd_len);

	switch(local_task.cmd)
	{
	    case CORE_CMD_TAP2GO_CMD:
	        Core_HandleTag2go(&local_task);
	        break;
		case CORE_CMD_ESL_UPDATA_REQUEST:
			Core_HandleEslUpdataReq(&local_task);
			break;
		case CORE_CMD_RF_SEND_FLASH_DATA:
			Core_HandleEslUpdataCmd(&local_task);
			memcpy((UINT8*)&local_task.flash_data_addr, (UINT8*)&tmp_data, sizeof(tmp_data));
			break;
		case CORE_CMD_WRITE_DATA_TO_FLASH:
			Core_HandleWriteESLData(&local_task);
			break;
		case CORE_CMD_ESL_HB_REQUEST:
			Core_HandleG3Heartbeat(&local_task);
			break;
		case CORE_CMD_RCREQ_REQUEST:
			Core_HandleRcReqRequest(&local_task);
			break;
		case CORE_CMD_SOFT_REBOOT:
			Core_HandleSoftReboot();
			break;
		case CORE_CMD_FT_RR_TXNULL:
			Core_HandleRummanTest(&local_task);
			break;
		case CORE_CMD_SCAN_BG:
			Core_HandleScanBG(&local_task);
			break;
		case CORE_CMD_FT_RF_BER:
			Core_HandleFTBerTest(&local_task);
			break;
		case CORE_CMD_QUERY_SOFT_VER:
			Core_HandleQuerySoftVer(&local_task);
			break;
		case CORE_CMD_QUERY_STATUS:
			Core_HandleQueryStatus(&local_task);
			break;
		case CORE_CMD_SET_DEBUG_LEVEL:
			Core_SetDebugLevel(&local_task);
			break;
		case CORE_CMD_SET_RF_LOG:
			Core_SetRfLog(&local_task);
			break;
//		case CORE_CMD_BACK_TO_IDLE:
//			Core_BackToIdel(&local_task);
//			break;
		case CORE_CMD_RF_TXRX:
			core_handle_rf_txrx(&local_task);
			break;
		case CORE_CMD_SCAN_DEVICE:   //0x1006
		    Core_HandleScanAck(&local_task);
		    break;
		case CORE_CMD_CALIBRATE_POWER:
		    Core_HandleCalibratePower(&local_task);
		    break;
        case CORE_CMD_CALIBRATE_FREQ:
            Core_HandleCalibrateFreq(&local_task);
            break;
		default:
		    memset(&local_task, 0 , sizeof(core_task_t));
			perr("Core_RxHandler() invalid cmd: 0x%04X\r\n", local_task.cmd);
			break;
	}
done:
	return;
}
extern INT32 wakeup_start(UINT32 addr, UINT32 len, UINT8 type);

void readHandleFnx(void)
{
    local_task.data_ptr = local_task.cmd_buf.buf;
    local_task.data_len = protocol_recv(local_task.data_ptr, sizeof(local_task.cmd_buf.buf));
    if(local_task.data_len > CORE_CMD_LEN){
        pinfo("Xmodem_RecvCallBack recv too big data(%d) to handle.\r\n", local_task.data_len);
        protocol_dataInit(local_task.data_ptr, sizeof(local_task.cmd_buf.buf));
    }else if((local_task.data_len > 0)&&(local_task.data_len <=TRANS_BUF_SIZE)){
        EP_DEBUG(("\r\n>>>EP1_OUT_Callback recv data len = %d.\r\n", local_task.data_len));
        Core_RxHandler();
        protocol_dataInit(local_task.data_ptr, sizeof(local_task.cmd_buf.buf));
    }else if(local_task.data_len < 0){
        EP_DEBUG(("\r\n>>>EP1_OUT_Callback recv error(%d)!\r\n", local_task.data_len));
        protocol_dataInit(local_task.data_ptr, sizeof(local_task.cmd_buf.buf));
    }else{
        EP_DEBUG(("\r\n>>>EP1_OUT_Callback.\r\n"));
    }
}

void Core_Mainloop(void)
{
    volatile uint32_t event = 0;
    while (1) {
        event = Event_PendCore(EVENT_ALL);
		if (USCI_status()){
			watchdog_clear();
			pinfo("feedWD");
		}

		if (0 == event)
		{
			continue;
		}
        if (RF_Status_carrierWave == rf_status){
            RF_carrierWave(false);
        }else {
            if (RF_Status_measureRSSI == rf_status){
                RF_measureRSSI(false);
            }
        }
        if(event & EVENT_COMMUNICATE_RX_HANDLE){
            readHandleFnx();
            Event_Clear(EVENT_COMMUNICATE_RX_HANDLE);
        }
        if(event & EVENT_TAP2GO){
            pinfo("t2g\r\n");
            Core_Tag2goRecMessage(&local_task);
            Event_Clear(EVENT_TAP2GO);
        }
        if (event & (EVENT_WRITE_DATA_AND_RF_SEND|EVENT_WRITE_ESL_DATA)){
            pdebug("cp2flash\r\n");
            memset((UINT8*)&tmp_data, 0, sizeof(tmp_data));
            memcpy((UINT8 *)&local_task.flash_data_len, local_task.cmd_buf.buf, sizeof(local_task.flash_data_len));
            //BSP_lowGPIO(DEBUG_TEST);		// LOW 21ms
         //   pinfo("local_task.flash_data_addr=0x%08X,local_task.flash_data_len=%d\r\n", local_task.flash_data_addr,local_task.flash_data_len);
            local_task.flash_data_addr=(UINT32)storage_malloc(local_task.flash_data_len);
            if(local_task.flash_data_addr!=NULL)
            {
                if(Core_SendAck(CORE_CMD_ACK, 0, NULL) == 1){
                    //BSP_lowGPIO(DEBUG_TEST);	//37ms
                	memcpy((UINT8*)&tmp_data, (UINT8*)&local_task.flash_data_addr, sizeof(tmp_data));
                    if(Core_RecvDataToFlash(local_task.flash_data_addr, local_task.flash_data_len) == 1){
    					if (event & EVENT_WRITE_DATA_AND_RF_SEND){
    						memset((UINT8*)&tmp_data, 0, sizeof(tmp_data));
    						Event_Set(EVENT_PARSE_DATA);
    					}
                    }else {
                    	storage_free((UINT8*)local_task.flash_data_addr);
                    }
                }
            } else {
                Core_SendAck(CORE_CMD_FLASH_ERROR, 0, NULL);
            }
            pdebug("cp2flash exit\r\n");
            Event_Clear((EVENT_WRITE_DATA_AND_RF_SEND|EVENT_WRITE_ESL_DATA));
        }
        if(event & EVENT_COMMUNICATE_SCAN_DEVICE){
            pinfo("core uart send ack.\r\n");
            local_task.ack_buf.buf[0] = Flash_readID(local_task.ack_buf.buf+1);
            local_task.ack_buf.buf[0] = local_task.ack_buf.buf[0]>0 ? 1 : 0;
        	memcpy (local_task.ack_buf.buf+3, (uint8_t*)&calib, sizeof(calib));
        	memcpy (local_task.ack_buf.buf+3+sizeof(calib), (uint8_t*)rf_tx_power, sizeof(rf_tx_power));
            local_task.ack_len = 1 + sizeof(calib) + sizeof(rf_tx_power);
            local_task.ack_ptr = local_task.ack_buf.buf;
            Core_SendAck(CORE_CMD_ACK, local_task.ack_len, local_task.ack_ptr);
            Event_Clear(EVENT_COMMUNICATE_SCAN_DEVICE);
        }
        if(event & EVENT_PARSE_DATA)
        {
        	pinfo("Parse\r\n");
            Core_ParseFlashData(local_task.flash_data_addr);
            Event_Clear(EVENT_PARSE_DATA);
        }


        if(event & EVENT_ESL_UPDATA)
        {
            esl_updata_t *this_updata;
            int32_t ret = 0;
            pinfo("updata esl start\r\n");
            local_task.flash_ack_addr = NULL;
            local_task.flash_ack_len = 0;// reset ack info
            this_updata=Core_Malloc(sizeof(esl_updata_t));
            if(this_updata == NULL)
            {
                pinfo("this_updata err.\r\n");//ggg debug
                perr("core malloc g3 updata table!\r\n");
                Core_SendAck(CORE_CMD_RAM_ERROR, 0, NULL);
            }
            else
            {
                this_updata->sid = 0;
                this_updata->data_addr = local_task.flash_data_addr;
                this_updata->data_len = local_task.flash_data_len;
                this_updata->ack_addr = 0;
                this_updata->ack_len = 0;
                ret = esl_updata(this_updata);
                pdebug("core updata esl exit.\r\n");
                if((this_updata->ack_addr!=NULL)&&(this_updata->ack_len!=0))
                {
                    Core_SendDataFromFlash(this_updata->ack_addr, this_updata->ack_len);//tx ack
                }else if ((this_updata->ack_addr==NULL)&&(this_updata->ack_len==0)){
                	pinfo("wk ack:%d, %x.\r\n", sizeof(st_protcolHead)+sizeof(this_updata->sid), (uint16_t)(this_updata->buf+4));

                	Core_SendData(this_updata->buf, sizeof(st_protcolHead)+sizeof(this_updata->sid));
                }else if (-1==ret){
                	Core_SendAck(CORE_CMD_ERROR, 0, NULL);
                }else{
                	//do nothing
                }
            }
            storage_free((UINT8*)this_updata->ack_addr);
            Core_Free(this_updata);
            storage_free((UINT8*)local_task.flash_data_addr);
            Event_Clear(EVENT_ESL_UPDATA);
        }

        if(event & EVENT_G3_HEARTBEAT)
        {
            g3_hb_table_t *hb_table;
            pinfo("core g3 hb start.\r\n");
            hb_table = Core_Malloc(sizeof(g3_hb_table_t));

            if(hb_table == NULL)
            {
                pinfo("core malloc overflow:%d\r\n", sizeof(g3_hb_table_t));
                Core_SendAck(CORE_CMD_RAM_ERROR, 0, NULL);
            }
            else
            {
                if(Core_SendAck(0x10f0, 0, NULL) == 1)
                {
                    heartbeat_mainloop(local_task.cmd_buf.buf, local_task.cmd_len, hb_table, Core_SendData);
                }
            }
            Core_Free(hb_table);
            pinfo("core g3 hb exit.\r\n");
            Event_Clear(EVENT_G3_HEARTBEAT);
        }

        if(event & EVENT_RC_REQ)
        {
            rcreq_table_t *rcreq_table;

            pinfo("core enter handle rc req\r\n");
            rcreq_table = Core_Malloc(sizeof(rcreq_table_t));

            if(rcreq_table == NULL)
            {
                perr("core malloc rcreq_table(size = %d)!\r\n", sizeof(rcreq_table_t));
                Core_SendAck(CORE_CMD_RAM_ERROR, 0, NULL);
            }
            else
            {
                if(RcReq_ParseCmd(local_task.cmd_buf.buf, local_task.cmd_len, rcreq_table) < 0)
                {
                    perr("RcReq_ParseCmd\r\n");
                    Core_SendAck(0x10FF, 0, NULL);
                }
                else
                {
                    RcReq_Mainloop(rcreq_table, Core_SendData);
                }
            }
            
            Core_Free(rcreq_table);
            pinfo("core exit handle rc req\r\n");
            Event_Clear(EVENT_RC_REQ);
        }
        if (event & EVENT_CALIBRATE_FREQ)
        {
            uint16_t ack;
            pinfo("calibrate frequency\r\n");
            ack = calibrate_freq(&local_task);
            Core_SendAck(ack, local_task.ack_len, local_task.ack_ptr);
            Event_Clear(EVENT_CALIBRATE_FREQ);
        }
        if (event & EVENT_CALIBRATE_POWER)
        {
            uint16_t ack;
            pinfo("calibrate power\r\n");
            ack = calibrate_power(&local_task);
            Core_SendAck(ack, local_task.ack_len, local_task.ack_ptr);
            Event_Clear(EVENT_CALIBRATE_POWER);
        }
        if(event & EVENT_SYSTEM_REBOOT)
        {
            pinfo("core system reboot.\r\n");
            BSP_Reboot();
            Event_Clear(EVENT_SYSTEM_REBOOT);
        }

        //EVENT_FT_BER
        if(event & EVENT_FT_BER)
        {
            INT32 fret = 0;
            pinfo("core ft ber.\r\n");
            fret = rft_check_ber_data(local_task.cmd_buf.buf, local_task.cmd_len);
            if(fret < 0)
            {
                Core_SendAck(CORE_CMD_PARA_ERROR, 0, NULL);
            }
            else if(fret != 0) //test board
            {
                Core_SendAck(CORE_CMD_ACK, 0, NULL);
                rft_ber(local_task.ack_buf.buf, sizeof(local_task.ack_buf.buf));
            }
            else //gold board
            {
                fret = rft_ber(local_task.ack_buf.buf, sizeof(local_task.ack_buf.buf));
                Core_SendAck(CORE_CMD_ACK, fret, local_task.ack_buf.buf);
            }
            Event_Clear(EVENT_FT_BER);
        }

        //EVENT_SCAN_BG
        if(event & EVENT_SCAN_BG)
        {
            INT32 fret = 0;
            pinfo("core scan bg\r\n");
            fret = rft_scan_bg(local_task.cmd_buf.buf, local_task.cmd_len, local_task.ack_buf.buf, sizeof(local_task.ack_buf.buf));
            if(fret <= 0)
            {
                Core_SendAck(CORE_CMD_ERROR, 0, NULL);
            }
            else
            {
                Core_SendAck(CORE_CMD_ACK, fret, local_task.ack_buf.buf);
            }
            Event_Clear(EVENT_SCAN_BG);
        }
        if(event & EVENT_RF_TXRX)
        {
            pinfo("Core rf test\r\n");
            local_task.ack_len = rf_txrx(local_task.cmd_buf.buf, local_task.cmd_len, local_task.ack_buf.buf, CORE_CMD_LEN);
            Core_SendData(local_task.ack_buf.buf, local_task.ack_len);
            Event_Clear(EVENT_RF_TXRX);
        }

        if(Core_GetQuitStatus() == 1)
        {
            pinfo("Core_ResetQuitStatus\r\n");
            Core_ResetQuitStatus();
        }

		if (USCI_status()){
			watchdog_clear();
		}

	}
}
