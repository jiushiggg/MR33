/* XDC module Headers */
#include <memery.h>
#include <xdc/std.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Error.h>

/* BIOS module Headers */
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <ti/sysbios/knl/Clock.h>

#include "sys_cfg.h"


static uint8_t coremem_inuse = 0;

//#pragma location = (0x11000000);
static uint8_t coremem[COREMEM_SIZE] = {0};
static IHeap_Handle ap_heap;

void *Core_Malloc(uint32_t size)
{
    if(coremem_inuse == 1)  {
        return NULL;
    } else if(size > COREMEM_SIZE)  {
        return NULL;
    } else {
//      coremem_inuse = 1;
        return coremem;
    }
}

void Core_Free(void *ptr)
{
    coremem_inuse = 0;
}




static void printHeapStats(IHeap_Handle heap)
{
    Memory_Stats stats;
    Memory_getStats(heap, &stats);
#ifdef xdc_target__isaCompatible_28
    pinfo("largestFreeSize = %ld", (ULong)stats.largestFreeSize);
    pinfo("totalFreeSize = %ld", (ULong)stats.totalFreeSize);
    pinfo("totalSize = %ld", (ULong)stats.totalSize);
#else
    pinfo("largestFreeSize = %d", stats.largestFreeSize);
    pinfo("totalFreeSize = %d", stats.totalFreeSize);
    pinfo("totalSize = %d", stats.totalSize);
#endif
}


void ap_heap_init(void)
{
    HeapMem_Params prms;
    HeapMem_Handle heap;
    Error_Block eb;
    Error_init(&eb);
    HeapMem_Params_init(&prms);

    prms.size = COREMEM_SIZE;
    prms.buf = (Ptr)coremem;
    prms.minBlockAlign = 8;
    heap = HeapMem_create(&prms, &eb);
    if (heap == NULL)
        while(1);
    ap_heap = HeapMem_Handle_upCast(heap);
    //printHeapStats(ap_heap);
}

void* ap_malloc(uint32_t size)
{
    Error_Block eb;
    Error_init(&eb);

    //request memory size is too large
    if(COREMEM_SIZE <= size)
        return NULL;

    //Ptr Memory_alloc(IHeap_Handle heap, SizeT size, SizeT align, Error_Block *eb)
    return Memory_alloc(ap_heap, size, 8, &eb);
}

void ap_free(void* ptr, uint32_t size)
{
    Memory_free(ap_heap, ptr, size);
    printHeapStats(ap_heap);
}
