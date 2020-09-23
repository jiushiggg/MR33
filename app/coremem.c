#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/Memory.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include "coremem.h"
#include "datatype.h"
#include "updata.h"
#include "debug.h"



/* used to align heapmem/heapbuf buffers */
typedef union {
    double d;
    long l;
    void *p;
} AlignData;


bool Ram_Read(uint32_t addr, uint8_t* src, uint32_t len);
bool Ram_Write(uint32_t addr, uint8_t* src, uint32_t len);
void *Core_Malloc(uint32_t size);
void Core_Free(void *ptr);





AlignData heapMemBuffer[COREMEM_SIZE / sizeof(AlignData)] = {0};
static HeapMem_Struct heapMemStruct;
static HeapMem_Handle task1Heap;
static IHeap_Handle heap;
Memory_struct buf[MEMORY_BUF_NUM] = {0};
static int8_t index = 0;


void Core_mallocInit(void)
{
	HeapMem_Params heapMemParams;
	HeapMem_Params_init(&heapMemParams);

	heapMemParams.size = COREMEM_SIZE;
	heapMemParams.minBlockAlign = 8;
	heapMemParams.buf = heapMemBuffer;
	HeapMem_construct(&heapMemStruct, &heapMemParams);
	task1Heap = HeapMem_handle(&heapMemStruct);
	heap = HeapMem_Handle_upCast(task1Heap);
	memset((uint8_t*)&buf, 0, sizeof(buf));
	index = 0;
}

void Core_printMallocStats(void)
{
    Memory_Stats stats;
    pinfo("start heap:%x", heap);
    Memory_getStats(heap, &stats);
    pinfo("Malloc:%d,%d,%d,%d\r\n", stats.largestFreeSize, stats.totalSize, stats.totalFreeSize, index);

}

void *Core_Malloc(uint32_t size)
{
    Memory_Stats stats;
    Memory_getStats(heap, &stats);
    pdebug("Malloc:%d,%d,%d,%d\r\n", stats.largestFreeSize, stats.totalSize, stats.totalFreeSize, index);
    if (stats.largestFreeSize < size){
    	return NULL;
    }
    if (index<0 || index>=MEMORY_BUF_NUM){
    	return NULL;
    }

	buf[index].ptr = Memory_alloc(heap, size, 0, NULL);
	buf[index].len = size;

	return buf[index++].ptr;
}

void Core_Free(void *ptr)
{
	uint8_t i = 0;
	if (NULL == ptr){
		return;
	}

	if (index<=0 || index>MEMORY_BUF_NUM) {
		pinfo("Core_Free error:%d\r\n", index);
		while(1);				//todo:need add watchdog
	}

	for (i = 0; i<MEMORY_BUF_NUM; i++){
		if (buf[i].ptr == ptr){
			break;
		}
	}

	if (i==MEMORY_BUF_NUM){
		while(1);				//todo:need add watchdog
	}

	index--;
	Memory_free(heap, buf[i].ptr, buf[i].len);
	//Core_printMallocStats();
	buf[i].ptr = NULL;
	buf[i].len = 0;

}

bool Ram_Read(uint32_t addr, uint8_t* src, uint32_t len)
{
    memcpy(src,(uint8_t *)addr,len);
   // pinfo("Ram_Read_done\r\n");
    return true;
}


bool Ram_Write(uint32_t addr, uint8_t* src, uint32_t len)
{
    memcpy((uint8_t*)addr,src, len);
  //  pinfo("Ram_Write_done\r\n");
    return true;
}


