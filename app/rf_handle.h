/*
 * rf_handle.h
 *
 *  Created on: 2020Äê10ÔÂ15ÈÕ
 *      Author: gaolongfei
 */

#ifndef APP_RF_HANDLE_H_
#define APP_RF_HANDLE_H_

#include "thread_rf.h"

typedef enum{
    PARSE_START,
    PARSE_DOING,
    PARSE_END
}em_parse_status;


typedef struct _rf_parse_st{
    em_parse_status status;
    uint16_t rf_cmd;
    uint32_t cmd_total_len;
    uint32_t cmd_left_len;
}rf_parse_st;

enum{
    SET_WK = (uint8_t) 0,
    GROUP_WK,
    FRAME1,
    SLEEP,
    UPDATA,
    QUERY,
    HANDLE_MAX_NUM
}em_cmd;

extern void rf_handle(rf_tsk_msg_t* msg);
extern uint8_t Core_GetQuitStatus(void);

#endif /* APP_RF_HANDLE_H_ */
