#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>
#include "event.h"
#include "watchdog.h"

#define EVENT_PEND_TIME ((WD_RESET_TIME_S/4+1)*1000000/Clock_tickPeriod)

Event_Handle protocol_eventHandle;
Event_Struct protocol_eventStruct;

static Semaphore_Struct  uart_sem_struct;
static Semaphore_Handle  uart_sem_handle;

Semaphore_Struct  RF_rxSemStruct;
Semaphore_Handle  RF_rxSemHandle;
Semaphore_Struct  RF_txSemStruct;
Semaphore_Handle  RF_txSemHandle;

void semaphore_uart_init(void)
{
    Semaphore_Params  recSemParam;
    Semaphore_Params_init(&recSemParam);
    recSemParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
    Semaphore_construct(&uart_sem_struct, 1, &recSemParam);
    uart_sem_handle = Semaphore_handle(&uart_sem_struct);
}
Bool uart_write_pend(uint32_t timeout)
{
    return Semaphore_pend(uart_sem_handle, timeout);
}
void uart_write_post(void)
{
    Semaphore_post(uart_sem_handle);
}

void Semaphore_RFInit(void)
{
    Semaphore_Params  SemParam;
    Semaphore_Params_init(&SemParam);
    SemParam.mode = ti_sysbios_knl_Semaphore_Mode_BINARY;
    Semaphore_construct(&RF_rxSemStruct, 0, &SemParam);
    RF_rxSemHandle = Semaphore_handle(&RF_rxSemStruct);

    Semaphore_construct(&RF_txSemStruct, 0, &SemParam);
    RF_txSemHandle = Semaphore_handle(&RF_txSemStruct);
}

void Semaphore_RFReconfig(uint8_t mode)
{
    Semaphore_Params  SemParam;
    Semaphore_Params_init(&SemParam);

	switch(mode){
		case SEMAPHORE_BINARY:
			SemParam.mode = ti_sysbios_knl_Semaphore_Mode_BINARY;
			break;
		case SEMAPHORE_COUNTING:
			SemParam.mode = ti_sysbios_knl_Semaphore_Mode_COUNTING;
			break;
		default:
			SemParam.mode = ti_sysbios_knl_Semaphore_Mode_BINARY;
			break;
	}

    Semaphore_construct(&RF_rxSemStruct, 0, &SemParam);
    RF_rxSemHandle = Semaphore_handle(&RF_rxSemStruct);
}

Bool RF_SemRecvPend(uint32_t timeout)
{
    return Semaphore_pend(RF_rxSemHandle, timeout);
}
void RF_SemRecvPost(void)
{
    Semaphore_post(RF_rxSemHandle);
}

Bool RF_SemSendPend(uint32_t timeout)
{
    return Semaphore_pend(RF_txSemHandle, timeout);
}
void RF_SemSendPost(void)
{
    Semaphore_post(RF_txSemHandle);
}
void Event_init(void)
{
    Event_Params eventParams;
    Event_Params_init(&eventParams);

    Event_construct(&protocol_eventStruct, &eventParams);
    protocol_eventHandle = Event_handle(&protocol_eventStruct);
}

uint32_t Event_Get(void)
{
    return Event_getPostedEvents(protocol_eventHandle);
}
void Event_Set(uint32_t event)
{
    Event_post(protocol_eventHandle, event);
}

void Event_Clear(uint32_t event)
{
//    event_flag ^= event;
}

uint32_t Event_GetStatus(void)
{
    return Event_getPostedEvents(protocol_eventHandle);
}

uint32_t Event_PendCore(uint32_t event)
{
    return Event_pend(protocol_eventHandle, 0, event, EVENT_PEND_TIME);

}
uint32_t taskDisable(void)
{
    return Task_disable();
}

void taskRestore(uint32_t key)
{
    Task_restore(key);
}

uint32_t swiDisable(void)
{
    return Swi_disable();
}

void swiRestore(uint32_t key)
{
    Swi_restore(key);
}
