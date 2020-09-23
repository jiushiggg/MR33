#ifndef _CORE_H_
#define _CORE_H_

#include <stdint.h>
#include "datatype.h"
#include "sys_cfg.h"
#include "protocol.h"
#include "tap2go.h"

#define CORE_CMD_SCAN_DEVICE            0x1006          //use uart 1step
#define CORE_CMD_ESL_UPDATA_REQUEST		0x1041
#define CORE_CMD_ESL_ACK_REQUEST		0x1042
#define CORE_CMD_FW_UPDATA_REQUEST		0x1044
#define CORE_CMD_ESL_HB_REQUEST			0x1048          //3step
#define CORE_CMD_RCREQ_REQUEST			0x1049
#define CORE_CMD_WRITE_DATA_TO_FLASH    0x1141
#define CORE_CMD_RF_SEND_FLASH_DATA     0x1142

#define CORE_CMD_SOFT_REBOOT			0x1000
#define CORE_CMD_QUERY_SOFT_VER			0x1001          //2step
#define CORE_CMD_QUERY_STATUS			0x1002
#define CORE_CMD_BACK_TO_IDLE			0x1003
#define CORE_CMD_SET_DEBUG_LEVEL		0x1004
#define CORE_CMD_SET_RF_LOG				0x1005

#define CORE_CMD_FT_RR_TXNULL			0x10A0
#define CORE_CMD_FT_RF_BER				0x10A1
#define CORE_CMD_SCAN_BG				0x10A2
#define CORE_CMD_RF_TXRX				0x10A3
#define CORE_CMD_CALIBRATE_POWER        0x10A4
#define CORE_CMD_CALIBRATE_FREQ         0x10A5

#define CORE_CMD_TAP2GO_CMD				0x10B0
#define CORE_CMD_SCAN_WKUP				0x10B1
#define CORE_CMD_ASS_ACK				0x10B2

#define CORE_CMD_FW_DATA				0x0400

#define CORE_CMD_ACK					0x10F0
#define CORE_CMD_BUSY					0x10F1
#define CORE_CMD_FLASH_ERROR			0x10F2
#define CORE_CMD_FLASH_ERROR2			0x10F3
#define CORE_CMD_RAM_ERROR				0x10F4
#define CORE_CMD_PARA_ERROR				0x10F5
#define CORE_CMD_ERROR					0x10FF

#define CORE_CMD_HB_ACK                 0x1020
#define CORE_CMD_ESL_UPLINK_ACK         0x1021
#define CORE_CMD_RC_UPLINK_ACK          0x1022
#define CORE_CMD_TAG2GO_MESSAGE         0x1023


/* Stack size in bytes */
#define GPRAM_BASE  0x11000000

#if defined(PCIE)
    #define TASK0_STACKSIZE   (2048-512)
#elif defined(AP_3)||defined(PCIE_SPI)
    #define TASK0_STACKSIZE   (2048-512)
#else

#endif


#define TASK0_ADDR  (GPRAM_BASE)

#define CORE_TASK_SIZE          (sizeof(core_task_t))
#define CORE_TASK_ADDR          (TASK0_ADDR+TASK0_STACKSIZE)

#define TRANS_BUF_ADDR          (CORE_TASK_ADDR+CORE_TASK_SIZE)

#if defined(PCIE)
    #define CORE_ACK_BUF_LEN UART_RECV_BUF
    #define CORE_CMD_BUF_LEN UART_RECV_BUF
#elif defined(AP_3)||defined(PCIE_SPI)
	#define CORE_ACK_BUF_LEN 100
	#define CORE_CMD_BUF_LEN 150
#else

#endif
#define CORE_CMD_LEN            CORE_CMD_BUF_LEN

#pragma pack(1)
typedef enum{
    TEST_FAILED          = (uint8_t)0,
    TEST_CONTINUE        = (uint8_t)1,
    TEST_PASS_SAVE       = (uint8_t)255
}EM_RESULT;

typedef enum{
    EM_UP   = (uint8_t)0,
    EM_DOWN = (uint8_t)1
}EM_DIRECTION;
typedef enum{
    EM_STOP   = (uint8_t)0,
    EM_START    = (uint8_t)1
}EM_FLG;

typedef struct st_calibration_freq{
    EM_RESULT result;
    uint8_t reserve;
    uint8_t channel;
    uint32_t channel_data;
    EM_DIRECTION flg;
    uint16_t fract_freq;
}st_calibration_freq;

typedef struct st_calibration_freq_ack{
    uint16_t frequency;
    uint16_t fractFreq;
}st_calibration_freq_ack;

typedef struct st_calibration_power{
    EM_RESULT result;
    uint8_t reserve;
    uint8_t power;
    uint32_t power_data;
    EM_DIRECTION flg;
    uint16_t power_set;
}st_calibration_power;

typedef struct st_calibration_power_ack{
    uint16_t power;
}st_calibration_power_ack;

typedef struct st_unmodulated_carrier{
    uint8_t c;
    int8_t  p;
    EM_FLG actor;
    uint8_t clear_c;
    uint8_t clear_p;
}st_unmodulated_carrier;

typedef struct st_unmodulated_carrier_ack{
    uint8_t c;
    int8_t  p;
    uint16_t frequency;
    uint16_t fractFreq;
    uint16_t power;
}st_unmodulated_carrier_ack;

typedef union  un_cmd_buf{
    UINT8                   buf[CORE_CMD_BUF_LEN];
    st_calibration_freq     calib_freq;
    st_calibration_power    calib_power;
    st_unmodulated_carrier  unmod_carrier;
    st_t2g_up_req        t2g;
    st_t2g_frame1        t2g_frame1;
    st_uplink_data       uplink_data;
    st_t2g_data          t2g_data;
    st_t2g_query         t2g_query;
    st_t2g_complete      t2g_complete;
}un_cmd_buf;


typedef union un_ack_buf{
    UINT8                   buf[CORE_ACK_BUF_LEN];
    st_calibration_freq_ack     freq;
    st_calibration_power_ack    power;
    st_unmodulated_carrier_ack  unmod_carrier;
    st_t2g_down_rsp             t2g_rsp;
    st_t2g_ack                  t2g_ack;
}un_ack_buf;


typedef struct core_task_t
{
    UINT16 cmd;
    UINT32 cmd_len;
    un_cmd_buf cmd_buf;
    UINT8 *data_ptr;
    INT32 data_len;
    UINT32 flash_data_addr;
    UINT32 flash_data_len;
    UINT32 flash_ack_addr;
    UINT32 flash_ack_len;
    UINT16 ack;
    UINT32 ack_len;
    un_ack_buf ack_buf;
    UINT8 *ack_ptr;
}core_task_t;

typedef struct st_flash_data{
	UINT32 flash_data_addr;
	UINT32 flash_data_len;
}st_flash_data;

#pragma pack()
extern void Core_Init(void);

extern void Core_TxHandler(void);
extern void Core_Mainloop(void);
extern uint8_t Core_GetQuitStatus(void);
extern void Core_ResetQuitStatus(void);
extern uint8_t Core_SendAck(uint16_t ack_cmd, uint32_t ack_len, uint8_t *ack_data);
extern core_task_t local_task;
extern st_flash_data tmp_data;

#endif
