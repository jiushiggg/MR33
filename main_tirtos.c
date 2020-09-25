/*
 * Copyright (c) 2016-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,

 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== main_tirtos.c ========
 */

#include <stdint.h>

/* POSIX Header files */

/* RTOS header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/Power/PowerCC26XX.h>
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "Board.h"
#include "thread_rf.h"
#include "thread_trans.h"
#include "cc2640r2_rf.h"
#include "timer.h"
#include "memery.h"
#include "event.h"
#include "debug.h"
#include "sys_cfg.h"
#include "watchdog.h"

/* Stack size in bytes */
#define GPRAM_BASE              0x11000000
#define TASK0_STACKSIZE         (2048- 512)
#define TASK0_ADDR              (GPRAM_BASE)

#define TASK1_STACKSIZE         (2048- 512)
#define TASK1_ADDR              (TASK0_ADDR+TASK0_STACKSIZE)

void app_init(void);
static void dongle_task_creat(void);

#pragma location = (GPRAM_BASE);
Char task0_Stack[TASK0_STACKSIZE];
Task_Struct task0_Struct;

#pragma location = (TASK1_ADDR);
Char task1_Stack[TASK1_STACKSIZE];
Task_Struct task1_Struct;

/*
 *  ======== main ========
 */
int main(void)
{
    Board_initGeneral();
//    Board_initWatchdog();
    rf_init();
    TIM_Init();
    Semaphore_RFInit();
    app_init();
    BIOS_start();    /* Start BIOS */
    return (0);

}

void app_init(void)
{
    Board_initSPI();
    Board_initUART();
    Board_initGPIO();
    watchdog_init();

#ifdef GPRAM_AS_RAM
    // retain cache during standby
    Power_setConstraint(PowerCC26XX_SB_VIMS_CACHE_RETAIN);
    Power_setConstraint(PowerCC26XX_NEED_FLASH_IN_IDLE);
#else
    // Enable iCache pre-fetching
    VIMSConfigure(VIMS_BASE, TRUE, TRUE);
    // Enable cache
    VIMSModeSet(VIMS_BASE, VIMS_MODE_ENABLED);
#endif //CACHE_AS_RAM

    Debug_SetLevel(DEBUG_LEVEL_INFO);
    debug_peripheral_init();

    //ap_heap_init();

    //Event_init();
    //Semphore_xmodemInit();

    dongle_task_creat();
}


static void dongle_task_creat(void)
{
    Task_Params taskParams_0;

    Task_Params_init(&taskParams_0);
    taskParams_0.arg0 = 1000000 / Clock_tickPeriod;
    taskParams_0.stackSize = TASK0_STACKSIZE;
    taskParams_0.stack = &task0_Stack;
    taskParams_0.priority = 2;
    Task_construct(&task0_Struct, (Task_FuncPtr)thread_transmit, &taskParams_0, NULL);


    Task_Params_init(&taskParams_0);
    taskParams_0.arg0 = 1000000 / Clock_tickPeriod;
    taskParams_0.stackSize = TASK1_STACKSIZE;
    taskParams_0.stack = &task1_Stack;
    taskParams_0.priority = 1;
    Task_construct(&task1_Struct, (Task_FuncPtr)thread_rf, &taskParams_0, NULL);
}
