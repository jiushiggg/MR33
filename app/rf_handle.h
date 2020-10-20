/*
 * rf_handle.h
 *
 *  Created on: 2020Äê10ÔÂ15ÈÕ
 *      Author: gaolongfei
 */

#ifndef APP_RF_HANDLE_H_
#define APP_RF_HANDLE_H_

typedef struct _rf_parse_st{
    em_parse_status status;
    uint16_t rf_cmd;
    uint32_t cmd_total_len;
    uint32_t cmd_left_len;
}rf_parse_st;



#endif /* APP_RF_HANDLE_H_ */
