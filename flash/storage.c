/*
 * storage.c
 *
 *  Created on: 2019Äê4ÔÂ26ÈÕ
 *      Author: ggg
 */
#include "storage.h"
#include "flash.h"
#include "coremem.h"
#include "updata.h"

typedef enum {
    STORETYPE_RAM    = (uint8_t)0,
    STORETYPE_FLASH = (uint8_t)1,
    STORETYPE_NUM    = (uint8_t)2
}em_StorageType;

static volatile em_StorageType storage_type = STORETYPE_RAM;

storeFuncTable flashrw={
.write  =   Flash_Write,
.read   =   Flash_Read,
.malloc =   Flash_Malloc,
.free   =   Flash_Free,
};

storeFuncTable ramrw={
	.write  =   Ram_Write,
	.read   =   Ram_Read,
	.malloc =   Core_Malloc,
	.free   =   Core_Free
};

storeFuncConfig storeConfig[STORETYPE_NUM] = {
{
.storeFuncPtr = &ramrw,
},
{
 .storeFuncPtr = &flashrw,
}

};

bool storage_write(uint32_t addr, uint8_t* src, uint32_t len)
{
    return storeConfig[storage_type].storeFuncPtr->write(addr, src, len);
}
bool storage_read(uint32_t addr, uint8_t* src, uint32_t len)
{
    return storeConfig[storage_type].storeFuncPtr->read(addr, src, len);
}

void *storage_malloc(uint32_t size)
{
	storage_type = size<=STORAGE_UPDATA_DATA_LEN ? STORETYPE_RAM : STORETYPE_FLASH;
    return storeConfig[storage_type].storeFuncPtr->malloc(size);
}

void storage_free(void* ptr)
{
    uint8_t i;
    storage_type=STORETYPE_FLASH;
    for (i = 0; i<MEMORY_BUF_NUM; i++){
        if (buf[i].ptr == ptr){
            storage_type=STORETYPE_RAM;
            break;
        }
    }

    return storeConfig[storage_type].storeFuncPtr->free(ptr);
}


