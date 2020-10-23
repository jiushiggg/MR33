/*
 * task_id.h
 *
 *  Created on: 2020Äê10ÔÂ15ÈÕ
 *      Author: gaolongfei
 */

#ifndef TASK_ID_H_
#define TASK_ID_H_

#define CORE_CMD_INVALID                0x0000          //invalid core cmd

#define CORE_CMD_SCAN_DEVICE            0x1006          //use uart 1step
#define CORE_CMD_ESL_UPDATA_REQUEST     0x1041
#define CORE_CMD_ESL_ACK_REQUEST        0x1042
#define CORE_CMD_FW_UPDATA_REQUEST      0x1044
#define CORE_CMD_ESL_HB_REQUEST         0x1048          //3step
#define CORE_CMD_RCREQ_REQUEST          0x1049

#define CORE_CMD_SOFT_REBOOT            0x1000
#define CORE_CMD_QUERY_SOFT_VER         0x1001          //2step
#define CORE_CMD_QUERY_STATUS           0x1002
#define CORE_CMD_BACK_TO_IDLE           0x1003
#define CORE_CMD_SET_DEBUG_LEVEL        0x1004
#define CORE_CMD_SET_RF_LOG             0x1005

#define CORE_CMD_FT_RR_TXNULL           0x10A0
#define CORE_CMD_FT_RF_BER              0x10A1
#define CORE_CMD_SCAN_BG                0x10A2
#define CORE_CMD_RF_TXRX                0x10A3
#define CORE_CMD_CALIBRATE_POWER        0x10A4
#define CORE_CMD_CALIBRATE_FREQ         0x10A5
#define CORE_CMD_FREE_RAM_BUF           0x10A6

#define CORE_CMD_SCAN_WKUP              0x10B1



#endif /* TASK_ID_H_ */
