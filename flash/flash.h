#ifndef __FLASH_H_
#define __FLASH_H_

#include <stdbool.h>
#include "datatype.h"

#define FLASH_INIT_OK		0
#define FLASH_INIT_ERR		1
#define FLASH_INIT_ERR_IO	2
#define FLASH_INIT_ERR_CK	3

#define MALLOC_FAIL			0x0

#define SECTOR_ERR			0x55
#define SECTOR_OK			0xFF
#define FLASH_USE_FLAG		0xAA
#define FLASH_BASE_ADDR		0x00


extern bool Flash_SetErrorSector(UINT16 sector);
extern UINT8 Flash_GetSectorStatus(UINT16 sector);
//flash��ʼ������������IO��ʼ����sector check���ɹ�����OK��ʧ�ܷ���ʧ����
extern UINT8 Flash_Init(void);
//��λ
extern void Flash_SoftReset(void);
//��flash�з���size��С�Ŀռ䣬�ɹ����ط���ռ����ʼ��ַ��ʧ�ܷ���MALLOC_FAIL
extern void* Flash_Malloc(UINT32 size);
//��flash��addr��ַд��ʼ��ַΪsrc������Ϊlen�����ݣ�addr��Ϊ�����ַ�����������Ѿ�����������
extern bool Flash_Write(UINT32 addr, UINT8* src, UINT32 len);
//��flash��addr��ַ��ȡ����Ϊlen�����ݵ�dst��addrΪ�����ַ
extern bool Flash_Read(UINT32 addr, UINT8* dst, UINT32 len);

extern bool Flash_writeInfo(UINT8* src, UINT32 len);
extern bool Flash_readInfo(UINT8* src, UINT32 len);
extern UINT8 Flash_calibInfoInit(void);
extern void Flash_Free(void *p);
extern void test_flash(void);
extern UINT8 Flash_readID(void *p);
#endif


