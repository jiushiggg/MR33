#include <stdint.h>
#include "flash.h"
#include "../peripheral/extern_flash.h"
#include "bsp.h"
#include "rftest.h"
#include "cc2640r2_rf.h"
#include "corefunc.h"


#define DATA_SECTER_CHECK  0
#define DATA_SECTER_INFO   1
#define DATA_SECTER_START  2
#define DATA_SECTER_END    510
#define DATA_SECTER_LOG    511

#define FLASH_SECTOR_NUM  	(DATA_SECTER_LOG+1)
#define FLASH_SECTOR_SIZE	4096
#define FLASH_PAGE_SIZE  	256


#define FLASH_INFO_BUF_LEN      10
#define FLASH_INFO_INDEX_LEN    1
#define FLASH_INFO_DATA_LEN     (FLASH_INFO_BUF_LEN - FLASH_INFO_INDEX_LEN)

#define SECTER_INFO_ADDR        (FLASH_SECTOR_SIZE * DATA_SECTER_INFO)
#define SECTER_INFO_DATA_ADDR    (SECTER_INFO_ADDR + FLASH_INFO_INDEX_LEN)

bool Flash_Write(UINT32 addr, UINT8* src, UINT32 len);
bool Flash_Read(UINT32 addr, UINT8* src, UINT32 len);
void* Flash_Malloc(UINT32 size);
void Flash_Free(void *p);

static UINT32 _sector = DATA_SECTER_START;


//#define FLASH_DBG

bool Flash_SetErrorSector(UINT16 sector)
{
	UINT8 flag = SECTOR_ERR;
	return CMD_PP(sector, (UINT32)&flag, sizeof(flag), VERIFY);
}

UINT8 Flash_GetSectorStatus(UINT16 sector)
{
	UINT8 status = 0;
	
	if(!CMD_FASTREAD(sector, (UINT32)&status, sizeof(status)))
	{
		status = 0;
	}
	
	return status;
}

#define FLASH_CHECK_ERR		0
#define FLASH_CHECK_OK		1
#define FLASH_CHECK_NEW		2

UINT8 Flash_Check(void)
{
	UINT8 ret = FLASH_CHECK_ERR;
	UINT8 flag = 0;
	UINT8 _buf[FLASH_SECTOR_NUM] = {0};
	UINT32 i, j;
	if (FlashOperationSuccess == CMD_FASTREAD(FLASH_BASE_ADDR, (UINT32)&flag, sizeof(flag)))
	{
		if (flag == FLASH_USE_FLAG)
		{
			//�Ѿ�ʹ�ù���flash����黵sector�����
			if (FlashOperationSuccess == CMD_FASTREAD(FLASH_BASE_ADDR, (UINT32)_buf, sizeof(_buf)))
			{
				j = 0;
				for (i = DATA_SECTER_START; i < DATA_SECTER_END + 1; i++)
				{
                    if (_buf[i] == SECTOR_ERR)
                    {
						j++;
					}
				}

				//����sector��С��sector������һ�룬�糬��������Ϊflash��
				if (j < ((DATA_SECTER_END-DATA_SECTER_START+1)/2))
				{
					ret = FLASH_CHECK_OK;
				}
			}
		}
		else
		{
			//��flash��дuse��־��������һ��sector��Ϊ��sector index
			if (CMD_SE(FLASH_BASE_ADDR) == FlashOperationSuccess)
			{
				flag = FLASH_USE_FLAG;
				if (CMD_PP(FLASH_BASE_ADDR, (UINT32)&flag, sizeof(flag), VERIFY) == FlashOperationSuccess)
				{
					ret = FLASH_CHECK_NEW;
				}
			}
		}
	}

	return ret;
}

void Flash_SoftReset(void)
{
	_sector = DATA_SECTER_START;
}

UINT8 Flash_readID(void *p)
{
	UINT16 *id = p;
	*id = CMD_RDID();
	pdebug("Flash ID =%x\r\n", *id);
	if ((*id!=FlashID) && (*id!=FlashID_GD)&& (*id!=FlashID_WB)&& (*id!=FlashID_PUYA))
	{
		return FLASH_INIT_ERR_IO;
	}
	return FLASH_INIT_OK;
}

UINT8 Flash_Init(void)
{
	UINT8 ret = 0;
	UINT16 id;
	
	init_nvs_spi_flash();
	//��ȡ
	id = CMD_RDID();
	log_print("Flash ID =%x\r\n", id);
	if ((id!=FlashID) && (id!=FlashID_GD)&& (id!=FlashID_WB)&& (id!=FlashID_PUYA))
	{
		return FLASH_INIT_ERR_IO;
	}

	ret = Flash_Check();
	if (ret == FLASH_CHECK_ERR)
	{
	    log_print("Flash Check failed.\r\n");
		return FLASH_INIT_ERR_CK;
	}
	else if (ret == FLASH_CHECK_NEW)
	{
	    log_print("new Flash.\r\n");
	}
	//ϵͳ���������õ�ǰsectorλ��
	_sector = DATA_SECTER_START;

    Flash_calibInfoInit();
    RF_configPower();
	return FLASH_INIT_OK;
}


void* Flash_Malloc(UINT32 size)
{
	UINT32 sector_num = 0;
	UINT16 i, j;
	UINT8 flag = 0;
	UINT32 ret = MALLOC_FAIL;
	UINT32 find_start_sector;

	if((size > 0) && (size < ((DATA_SECTER_END-DATA_SECTER_START+1)*FLASH_SECTOR_SIZE)))
	{
		//������Ҫ�����sector����
		sector_num = size / FLASH_SECTOR_SIZE;
		if ((size % FLASH_SECTOR_SIZE) != 0)
		{
			sector_num += 1;
		}

		find_start_sector = _sector;
find_loop:
		//�ڵ�ǰ��ַ��ʼ����sector��ֱ���ҵ�������sector_num��sector
		j = 0;
		for (i = find_start_sector; i < DATA_SECTER_END; i++)
		{
			if (CMD_SE(i*FLASH_SECTOR_SIZE) == FlashOperationSuccess)
			{
				j += 1;
			}
			else
			{
				Flash_SetErrorSector(i);
				j = 0;
			}

			if (j == sector_num)
			{
				//�ɹ��ҵ������ò��ҳɹ���־flag����¼�´�malloc����ʼsector
				flag = 1;
				_sector = i + 1;
				ret = (i + 1 - sector_num) * FLASH_SECTOR_SIZE;
				break;
			}
		}

		if (flag == 0)
		{
			//��һ�ֵ�
			find_start_sector = DATA_SECTER_START;
			flag = 2;
			goto find_loop;
		}
	}
	BSP_Delay1MS(15);
	return (void*)ret;
}
void Flash_Free(void *p)
{

}


bool Flash_Write(UINT32 addr, UINT8* src, UINT32 len)
{
	bool ret = FALSE;
	UINT32 left_len = len;
	UINT32 w_addr = addr;
	UINT8 *ptr = src;

	if((w_addr>= ((DATA_SECTER_END+1)*FLASH_SECTOR_SIZE)) || (w_addr < (DATA_SECTER_START*FLASH_SECTOR_SIZE)))
	{
		goto done;
	}

	//�ж�addr�Ƿ��ڿ�д��Χ��
	if (w_addr > (_sector*FLASH_SECTOR_SIZE))
	{
	    goto done;
	}

    if (CMD_PP(w_addr, (UINT32)ptr, left_len, NOT_VERIFY) == FlashOperationSuccess){
        ret = TRUE;
    }

done:
	return ret;
}

bool Flash_Read(UINT32 addr, UINT8* dst, UINT32 len)
{
    CMD_FASTREAD(addr, (UINT32)dst, len);
	return TRUE;
}

static ReturnMsg info_write(UINT8 flg)
{
    calib.flg = flg;
    calib.frequency_offset=0;
    calib.power_offset = 0;
    CMD_SE(SECTER_INFO_ADDR);
    return CMD_PP(SECTER_INFO_ADDR, (WORD)&calib, sizeof(calib), VERIFY);
}


bool Flash_writeInfo(UINT8* src, UINT32 len)
{
    UINT8 ret = FALSE;

    CMD_SE(SECTER_INFO_ADDR);
    if (CMD_PP(SECTER_INFO_ADDR, (UINT32)src, len, VERIFY) == FlashOperationSuccess){
        ret = TRUE;
    }else {
        info_write(0);
    }

    return ret;
}
bool Flash_readInfo(UINT8* src, UINT32 len)
{
    CMD_FASTREAD(SECTER_INFO_DATA_ADDR, (UINT32)src, len);
    return TRUE;
}


UINT8 Flash_calibInfoInit(void)
{
    CMD_FASTREAD(SECTER_INFO_ADDR, (UINT32)&calib, sizeof(calib));
    if (FLASH_USE_FLAG != calib.flg){
        info_write(FLASH_USE_FLAG);
    }
    return TRUE;
}
#ifdef FLASH_APP_TEST
#include <string.h>
#define TEST_LEN    4096
uint8_t coremem[TEST_LEN] = {0};
uint8_t coremem1[TEST_LEN] = {0};
uint32_t j = 0;
void test_flash(void)
{
	uint32_t address;
	while(1){
	    UINT16 i=0;

	    //BSP_Delay1MS(15);
	    for (i=0; i<TEST_LEN; i++){
	        coremem[i] = j+1;
	    }
	    memset(coremem1, 0, TEST_LEN);

	    //CMD_SE(index*FLASH_SECTOR_SIZE);
	    address = (uint32_t)Flash_Malloc(TEST_LEN);
	    Flash_Write(address, coremem , TEST_LEN);
	    Flash_Read(address, coremem1, TEST_LEN);

	    if (0 != memcmp(coremem, coremem1, TEST_LEN)){
	    	pinfo("error:%d, address:%x", j, address);
	    }
	    if (0 == j%100){
	    	pinfo("flash test running:%d times, %x\r\n",j, address);
	    }
	    j++;
	}
}

#endif

