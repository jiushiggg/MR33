/*
 * storage.h
 *
 *  Created on: 2019Äê4ÔÂ26ÈÕ
 *      Author: ggg
 */

#ifndef FLASH_STORAGE_H_
#define FLASH_STORAGE_H_
#include <stdint.h>
#include <stdbool.h>

typedef bool (*STORE_write)(uint32_t addr, uint8_t* src, uint32_t len);
typedef bool (*STORE_read)(uint32_t addr, uint8_t* src, uint32_t len);
typedef void* (*STORE_malloc)(uint32_t size);
typedef void  (*STORE_free)(void *ptr);

typedef struct storeFuncTable{
     STORE_write                    write;
     STORE_read                     read;
     STORE_malloc                   malloc;
     STORE_free                     free;
}storeFuncTable;

typedef struct storeFuncConfig{
    storeFuncTable const *storeFuncPtr;
}storeFuncConfig;

extern bool storage_write(uint32_t addr, uint8_t* src, uint32_t len);
extern bool storage_read(uint32_t addr, uint8_t* src, uint32_t len);
extern void *storage_malloc(uint32_t size);
extern void storage_free(void* ptr);

#endif /* FLASH_STORAGE_H_ */
