/*
 * thread_rf.h
 *
 *  Created on: 2020Äê9ÔÂ23ÈÕ
 *      Author: gaolongfei
 */

#ifndef _THREAD_RF_H_
#define _THREAD_RF_H_

typedef struct _rf_tsk_msg {
    uint16_t type;
    uint16_t id;
    uint32_t len;
    uint8_t* buf;
    uint32_t size;
    void* extra;
}rf_tsk_msg_t;

extern int8_t forward_msg_rfthread(uint16_t id, uint8_t* data, uint32_t length, uint32_t size, uint32_t storage);

#endif /* _THREAD_RF_H_ */
