#ifndef _BSP_H
#define _BSP_H

#include <ti/sysbios/knl/Clock.h>

void wait_ms(uint32_t ms);

extern void BSP_Delay1S(uint32_t n);
extern void BSP_Delay1MS(uint32_t n);
extern void BSP_Delay100US(uint32_t n);
extern void BSP_Delay10US(uint32_t n);
extern void BSP_Delay1US(uint32_t n);
extern void exceptionHandler(void);

#endif
