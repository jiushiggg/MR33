#ifndef UPDATA1_H_
#define UPDATA1_H_
#include "datatype.h"
#include "updata.h"

#define SLEEP_FRAME_CNT   2
extern UINT16 get_missed_sn_r(UINT8 *data, UINT8 offset);
extern UINT8 check_failed_pkg_r(UINT16 sn, UINT8 *buf, UINT8 failed_num);

extern UINT16 m1_init_data(UINT32 addr, UINT32 len, updata_table_t *table);
extern UINT8 m1_updata_loop(updata_table_t *table);
extern INT32 m1_send_sleep(updata_table_t *table, UINT8 timer);
extern void m1_make_ack(updata_table_t *table, UINT32 *ack_addr, UINT32 *ack_len, UINT8 sn_offset);
extern void m1_make_new_ack(updata_table_t *table, UINT32 *ack_addr, UINT32 *ack_len, UINT8 sn_offset);
extern INT32 updata_m1(UINT32 addr, UINT32 len, UINT32 *ack_addr, UINT32* ack_len, updata_table_t *table);
extern INT32 m1_sleep_all(updata_table_t *table);

#endif
