#include "bsp.h"
#include <stdio.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/devices/cc26x0r2/driverlib/sys_ctrl.h>

void exceptionHandler(void)
{
#if defined(ENABLE_EXCEPTION)
    SysCtrlSystemReset();
#else
    while(1);
#endif
}

void wait_ms(uint32_t ms)
{
    Task_sleep(ms*1000/Clock_tickPeriod);
}


void BSP_Delay1MS(uint32_t delayMs)
{
    volatile uint32_t j;
    /* by experimination, this is in ms (approx) */
    for (j = 0; j < 4010 * delayMs; j++)
    {
        asm(" NOP");
    }
}

void BSP_Delay100US(uint32_t delayUs)
{
    volatile  uint32_t j;
    for (j = 0; j < 400 * delayUs; j++)
    {
        asm(" NOP");
    }
}

void BSP_Delay10US(uint32_t delayUs)
{
    volatile  uint32_t j;
    for (j = 0; j < 40 * delayUs; j++)
    {
        asm(" NOP");
    }
}

void BSP_Delay1US(uint32_t delayUs)
{
    volatile  uint32_t j;
    for (j = 0; j < 4 * delayUs; j++)
    {
        asm(" NOP");
    }
}

