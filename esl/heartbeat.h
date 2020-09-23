#ifndef _HEARTBEAT_H_
#define _HEARTBEAT_H_

#include "datatype.h"
#include "updata.h"

#define UPLINK_BUF_NUM	7

#define    CTRL_HEARTBEAT        (uint8_t)0x80
#define    CTRL_ESL_UPLINK_DATA  (uint8_t)0x70
#define    CTRL_RCREQ         (uint8_t)0x03 //rc uplink
#define    CTRL_STREQ         (uint8_t)0x05 //signal test

#pragma pack(1)

#define BUF_NUM		2
#define ONCE_TRANS_HB_NUM 		(ESL_HB_NUM/BUF_NUM)
#define ONCE_TRANS_HB_BUF 		(HB_ESL_BUF_SIZE/BUF_NUM)


typedef struct st_hb{
	st_protcolHead head;
	uint16_t num;
	uint8_t hb_buf[ONCE_TRANS_HB_BUF];
}st_hb;

typedef struct g3_hb_table_t
{
	UINT8 id[4];
	UINT8 channel;
	UINT16 recv_bps;
	UINT8 recv_len;
	
	UINT32 interval;
	UINT32 timeout;
	UINT32 lenout;
	UINT32 numout;
	UINT32 loop;
	UINT32 apid;
	
	st_hb data_buf[BUF_NUM];
	uint8_t uplink_buf[UPLINK_BUF_NUM][27];
}g3_hb_table_t;


#pragma pack()


UINT8 set_timer(INT32 timeout);
UINT8 check_timer_timeout(UINT8 timer);
void close_timer(UINT8 timer);
void heartbeat_init(void);
INT32 heartbeat_mainloop(UINT8 *pCmd, INT32 cmdLen, g3_hb_table_t *table, UINT8 (*uplink)(UINT8 *src, UINT32 len));
INT32 common_recv(g3_hb_table_t *table, INT32 (*check_fun)(UINT8 *ptr, UINT8 len));
INT32 common_recv_parse_cmd(UINT8 *pCmd, INT32 cmdLen, g3_hb_table_t *table);

#endif
