#ifndef _COREMEM_H_
#define _COREMEM_H_

#include <stdbool.h>

#define G_PKG_BIT_MAP_LEN   400           //RAM中包号存储区的大小
#define MEMORY_BUF_NUM  5

typedef struct Memory_struct{
    void * ptr;
    uint32_t len;
}Memory_struct;


extern Memory_struct buf[MEMORY_BUF_NUM];

extern void Core_mallocInit(void);
extern void *Core_Malloc(uint32_t size);
extern void Core_Free(void *ptr);
extern void Core_printMallocStats(void);
extern bool Ram_Read(uint32_t addr, uint8_t* src, uint32_t len);
extern bool Ram_Write(uint32_t addr, uint8_t* src, uint32_t len);

#endif
