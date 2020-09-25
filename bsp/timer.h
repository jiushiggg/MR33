#ifndef _TIMER_H_
#define _TIMER_H_
#include <stdint.h>


#define    TIMER_UP_CNT   0
#define    TIMER_DOWN_CNT 1

#define WAIT_FOREVER	BIOS_WAIT_FOREVER

typedef enum{
    TIME_COUNTING=0,
    TIME_OUT=1,
    TIME_NONE
}emTimeCheck;

typedef enum{
    TIMER0=0,
    TIMER_UNKNOW
}emTimerSn;


typedef enum{
    TIMER_ONCE=0,
    TIMER_PERIOD=1,
    TIMER_UNKNOW_MODE
}emTimerMode;

typedef void (*TIM_ISRHandle)(uint8_t n);

extern void TIM_Init(void);
extern uint8_t TIM_Open(uint32_t nms, uint16_t cnt, uint16_t direction, emTimerMode mode);
extern void TIM_Close(uint8_t t);
extern uint8_t TIM_CheckTimeout(uint8_t t);
extern int32_t TIM_GetCount(uint8_t t);
extern void TIM_SetSoftTimeout(uint8_t t);
extern uint8_t   getTimerNum(void);
extern uint32_t TIM_GetTicks(void);
extern void TIM_SetCallback(uint8_t n, TIM_ISRHandle p_fnx);
#endif

