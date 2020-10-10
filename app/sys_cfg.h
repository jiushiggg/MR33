#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_
#include "debug.h"


#define COREMEM_SIZE    (1024)


//debug info output peripheral
#define DEBUG_PERIPHERAL DEBUG_SPI


#define MAX_TRANS_PACKET_NUM    48  //137*1400/512/8
#define MAX_PACKET_NUM          384

#endif
