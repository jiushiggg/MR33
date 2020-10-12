#ifndef _EVENT_H_
#define _EVENT_H_

#include <ti/sysbios/knl/Clock.h>

#define EVENT_NONE				0
#define EVENT_PARSE_DATA		((uint32_t)1<<0) //1
#define EVENT_ESL_UPDATA		((uint32_t)1<<1) //2
#define EVENT_FW_UPDATA			((uint32_t)1<<2) //4
#define EVENT_TAP2GO            ((uint32_t)1<<3)
#define EVENT_G3_HEARTBEAT		((uint32_t)1<<6)
#define EVENT_RC_REQ			((uint32_t)1<<7)
#define EVENT_SCAN_WKUP			((uint32_t)1<<11)
#define EVENT_CALIBRATE_POWER   ((uint32_t)1<<15)
#define EVENT_CALIBRATE_FREQ    ((uint32_t)1<<16)
#define EVENT_RF_TXRX			((uint32_t)1<<17)
#define EVENT_SCAN_BG			((uint32_t)1<<18)
#define EVENT_FT_BER			((uint32_t)1<<19)
#define EVENT_SYSTEM_REBOOT		((uint32_t)1<<20)

#define EVENT_COMMUNICATE_RX_HANDLE         ((uint32_t)1<<22) //
#define EVENT_WRITE_DATA_AND_RF_SEND       ((uint32_t)1<<23) //
#define EVENT_COMMUNICATE_TX_FROM_FLASH     ((uint32_t)1<<24) //
#define EVENT_COMMUNICATE_SCAN_DEVICE       ((uint32_t)1<<26) //
#define EVENT_WRITE_ESL_DATA	((uint32_t)1<<27)


#define EVENT_RF_ERR			(1<<30)
#define EVENT_ALL   0xFFFFFFFF


#define SEMAPHORE_COUNTING 0
#define	SEMAPHORE_BINARY 1

#define EVENT_WAIT_FOREVER   ti_sysbios_BIOS_WAIT_FOREVER

#define EVENT_WAIT_US(n)     ((uint32_t)n/Clock_tickPeriod)


typedef enum _eventStatus{
    EVENT_BUSY,
    EVENT_IDLE,
}eventStatus;

extern void Event_init(void);

extern uint32_t Event_Get(void);
extern void Event_Set(uint32_t event);
extern void Event_Clear(uint32_t event);
extern uint32_t Event_GetStatus(void);
extern uint32_t Event_PendCore(uint32_t event);


extern void semaphore_uart_init(void);
extern Bool uart_write_pend(uint32_t timeout);
extern void uart_write_post(void);
extern uint32_t taskDisable(void);
extern void taskEnable(uint32_t key);
extern uint32_t swiDisable(void);
extern void swiRestore(uint32_t key);
extern void Semaphore_RFInit(void);
extern Bool RF_SemRecvPend(uint32_t timeout);
extern void RF_SemRecvPost(void);
extern Bool RF_SemSendPend(uint32_t timeout);
extern void RF_SemSendPost(void);
extern void Semaphore_RFReconfig(uint8_t mode);

#endif
