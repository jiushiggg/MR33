/*
 * thread_main.h
 *
 *  Created on: 2020Äê9ÔÂ23ÈÕ
 *      Author: gaolongfei
 */

#ifndef APP_THREAD_TRANS_H_
#define APP_THREAD_TRANS_H_
#include <ti/drivers/utils/list.h>

typedef enum _trans_buf_status{
    TRANS_BUF_USING   =   (uint8_t)0,
    TRANS_BUF_IDLE,
}trans_buf_status;

typedef struct _trans_struct {
    List_Elem elem;
    uint16_t buf_total;
    uint16_t buf_len;
    uint8_t * buf;
    trans_buf_status buf_status;
    uint8_t buf_index;
} trans_struct;
extern int uart_data_send(uint8_t type, uint16_t id, uint8_t* data, uint32_t length, void* extra);

#endif /* APP_THREAD_TRANS_H_ */
