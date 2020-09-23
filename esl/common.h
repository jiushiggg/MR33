#ifndef _G3_COMMOM_H_
#define _G3_COMMOM_H_

#include "datatype.h"

#define CTRL_SLEEP			(7 << 5)
#define CTRL_LINK_QUERY		(3 << 5)		

extern INT32 make_sleep_data(UINT8 *eslid, UINT8 x, UINT8 *pdata, UINT8 len);
extern void g3_make_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len);
extern UINT8 g3_check_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len);
extern void g2_make_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len);
extern UINT8 g2_check_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len);
extern UINT16 cal_crc16(UINT8 ctrl, const UINT8 *eslid, const UINT8 *pdata, UINT8 len);
extern void t2g_make_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len);
extern UINT8 t2g_check_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len);
extern INT32 t2g_make_sleep_data(UINT8 *eslid, UINT8 x, UINT8 *pdata, UINT8 len);
#endif

