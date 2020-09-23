#include "cc2640r2_rf.h"
#include "sleep.h"
#include "storage.h"
#include "debug.h"
#include "updata.h"
#include "data.h"
#include "g1updata.h"
#include "common.h"
#include "bsp.h"
#include "core.h"

#define OFFSET_SLEEP_DATA	17
#define SIZE_MAX_ESL_BUF	64

#define SEND_QUERY_TIME		2	//2ms
#define RETRY_TIMES			2
#define FRAME1_TIME			20  //20ms
#define SEND_SLEEP_TIME		1	//1ms
#define SEND_DATA_TIME		85	//850us

static UINT32 sleep_addr = 0;
static INT32 sleep_len = 0;

/* sleep paras */
static UINT16 sleep_datarate = 0;
static UINT8 sleep_power = 0;
static UINT8 sleep_mode = 0;
static UINT8 sleep_interval = 0;
static UINT8 sleep_idx = 0;
static UINT8 sleep_times = 0;
static UINT8 sleep_default_len = 0;
static UINT16 sleep_num = 0;

/* one sleep data */
static UINT8 sleep_id[4] = {0};
static UINT8 sleep_channel = 0;
static UINT8 sleep_data[SIZE_MAX_ESL_BUF] = {0};
static UINT8 sleep_data_len = 0;

static INT32 sleep_mode0(void)
{
	INT32 ret = 0;
	INT32 i, j;
	UINT32 cur = 0;
	INT32 read_len = 0;
	volatile INT8 prev_channel=RF_FREQUENCY_UNKNOW;
	sleep_times = 1;

	for(j = 0; j < sleep_times; j++)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("sleep_mode0 quit2\r\n");
			break;
		}
		
		ret = 0;
		cur = sleep_addr+OFFSET_SLEEP_DATA;
		
		for( i = 0; ((i<sleep_num) && (cur<sleep_addr+sleep_len)); i++)
		{
			if(Core_GetQuitStatus() == 1)
			{
				pdebug("sleep_mode0 quit1\r\n");
				break;
			}
			
			read_len = get_one_data(cur, sleep_id, &sleep_channel, &sleep_data_len, sleep_data, sizeof(sleep_data));
			if(read_len <= 0)
			{
				ret = -1;
				perr("sleep mode0, g1 get data\r\n");
				break;
			}
			
			if(sleep_data_len == 0)
			{
				sleep_data_len = sleep_default_len;
				make_sleep_data(sleep_id, sleep_idx, sleep_data, sleep_data_len);
			}

	        if (sleep_channel != prev_channel){
	            set_frequence(sleep_channel);
	        }
	        prev_channel = sleep_channel;
	        send_data(sleep_id, sleep_data, sleep_data_len, 1000);
//			send_data(sleep_id, sleep_data, sleep_data_len, sleep_channel, 1000);
			
			pdebug("sleep %02X-%02X-%02X-%02X, channel=%d, datalen=%d: ", \
					sleep_id[0], sleep_id[1], sleep_id[2], sleep_id[3], sleep_channel, sleep_data_len);
			pdebughex(sleep_data, sleep_data_len);
			
			BSP_Delay1MS(sleep_interval);
			
			cur += read_len;
		}
	}

	//set_cmd_stby();	
	
	return ret;
}

INT32 sleep_init(UINT32 addr, INT32 len, updata_table_t *table)
{
	INT16 valid_slp_num;

	if((addr == 0) || (len == 0))
	{
		return -1;
	}
	
	sleep_addr = addr;
	sleep_len = len;
	
	storage_read(addr, (UINT8 *)&sleep_datarate, 2);
	storage_read(addr+2, &sleep_power, 1);
	storage_read(addr+3, &sleep_mode, 1);
	storage_read(addr+4, &sleep_interval, 1);
	storage_read(addr+5, &sleep_idx, 1);
	storage_read(addr+6, &sleep_times, 1);
	storage_read(addr+7, &sleep_default_len, 1);
	if((sleep_default_len==0) || (sleep_default_len > SIZE_MAX_ESL_BUF))
	{
		sleep_default_len = 26;
	}
	storage_read(addr+15, (UINT8 *)&sleep_num, 2);
	if (table->esl_num < 6){
		valid_slp_num = (table->esl_work_duration*1000 - \
						FRAME1_TIME -\
						table->max_esl_pkg_num*(table->tx_interval+1) - \
						table->esl_num*(table->deal_duration + SEND_QUERY_TIME)*RETRY_TIMES	-\
						table->esl_num*SEND_SLEEP_TIME	\
						)*2/3;		//1.5ms send sleep
//		pinfo("para:%d,%d,%d,%d,%d\r\n", table->esl_work_duration, table->esl_num, table->max_esl_pkg_num, table->deal_duration,table->tx_interval);
//		pinfo("cal:%d-%d-%d-%d-%d\r\n", table->esl_work_duration*1000, FRAME1_TIME, \
//			  	  	  	  	  	  	 table->max_esl_pkg_num*(table->tx_interval+1), \
//									 table->esl_num*(table->deal_duration + SEND_QUERY_TIME)*RETRY_TIMES, \
//									 table->esl_num*SEND_SLEEP_TIME);
	}else {
		valid_slp_num = (table->esl_work_duration*1000 - \
						FRAME1_TIME - \
						(table->esl_num * table->max_esl_pkg_num*SEND_DATA_TIME+50)/100 - \
						table->esl_num*(table->deal_duration + SEND_QUERY_TIME)*RETRY_TIMES	-\
						table->esl_num*SEND_SLEEP_TIME	  \
						)*2/3;		//1.5ms send sleep
//		pinfo("para:%d,%d,%d,%d\r\n", table->esl_work_duration, table->esl_num, table->max_esl_pkg_num, table->deal_duration);
//		pinfo("cal:%d-%d-%d-%d-%d\r\n", table->esl_work_duration*1000, FRAME1_TIME, \
//			  	  	  	  	  	  	  	(table->esl_num * table->max_esl_pkg_num*SEND_DATA_TIME+50)/100, \
//										table->esl_num*(table->deal_duration + SEND_QUERY_TIME)*RETRY_TIMES, \
//										table->esl_num*SEND_SLEEP_TIME);

	}

	if (valid_slp_num > 0){
		sleep_num = valid_slp_num<sleep_num ? valid_slp_num : sleep_num;
	}else{
		sleep_num = 0;
	}
	return 0;
}

void sleep_exit(void)
{
	sleep_addr = 0;
	sleep_len = 0;
}

INT32 sleep_start(const UINT32 addr, const UINT32 len, updata_table_t *table)
{
	INT32 ret = 0;

	if(sleep_init(addr, len, table) < 0)
	{
		ret = -1;
		goto done;
	}

	pdebug("sleep:datarate=%d,power=%d,mode=%d,interval=%d,idx=%d,times=%d,num=%d,default_len=%d.\r\n", \
			sleep_datarate, sleep_power, sleep_mode, sleep_interval, sleep_idx, sleep_times, sleep_num, sleep_default_len);
	
	if(sleep_num == 0)
	{
		goto done;
	}
	
	set_power_rate(sleep_power, sleep_datarate);
	
	switch(sleep_mode)
	{
		case 0: // 0 is default
		default:
			if(sleep_mode0() < 0)
			{
				ret = -2;
			}
		break;
	}
	
	sleep_exit();
done:
	return ret;
}



