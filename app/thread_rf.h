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

extern Void *thread_rf(UArg arg0, UArg arg1);

#endif /* _THREAD_RF_H_ */
