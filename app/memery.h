#ifndef _COREMEM_H_
#define _COREMEM_H_

#include <stdbool.h>
#include <stdint.h>

#define G_PKG_BIT_MAP_LEN   400           //RAM中包号存储区的大小
#define MEMORY_BUF_NUM  5

typedef struct _Memory_struct{
    void * ptr;
    uint32_t len;
}Memory_struct;


extern Memory_struct buf[MEMORY_BUF_NUM];

extern void *Core_Malloc(uint32_t size);
extern void Core_Free(void *ptr);
extern void ap_heap_init(void);
extern void* ap_malloc(uint32_t size);
extern void ap_free(void* ptr, uint32_t size);
extern void ap_heap_stats(void);

#endif
