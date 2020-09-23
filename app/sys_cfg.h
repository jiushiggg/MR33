#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

#include "debug.h"

//#define   AP_3            //Linux <---SPI---> Dongle
//#define   PCIE            //Linux <---UART----> Dongle
//#define   PCIE_SPI		//Linux <---SPI---> Dongle

#if defined(PCIE)
    #define PROTOCOL_TYPE   PROTOCOL_XMODEM
    #define DEBUG_PERIPHERAL    DEBUG_SPI
#elif defined(AP_3)
    #define PROTOCOL_TYPE   PROTOCOL_SPI
    #define DEBUG_PERIPHERAL    DEBUG_UART
#elif defined(PCIE_SPI)
    #define PROTOCOL_TYPE   PROTOCOL_SPI
    #define DEBUG_PERIPHERAL    DEBUG_UART
#else

#endif

//#define TAP2GO			 //if Support Tap2go
#define ESL_LOCATION	    //if Support ESL Geolocation
#endif
