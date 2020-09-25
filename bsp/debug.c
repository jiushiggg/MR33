#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "debug.h"
#include "bsp_spi.h"
#include "bsp_uart.h"
#include "bsp.h"
#include "sys_cfg.h"

volatile uint32_t s_debug_level = DEBUG_LEVEL_DFAULT;

#define LOG_SIZE    64

unsigned char debug_buf[LOG_SIZE];
int (*debugWrite)(void  *buf, uint16_t len) ;


void debug_peripheral_init(void)
{
    em_debug_peripheral type = DEBUG_PERIPHERAL;

    SPI_bsp_init(NULL, debug_buf);
    switch (type){
        case DEBUG_SPI:
            debugWrite = SPI_bsp_send;
            break;
        case DEBUG_UART:
            break;
        default:
            break;
    }
}
uint8_t Debug_GetLevel(void)
{
    return s_debug_level;
}

void Debug_SetLevel(uint8_t new_level)
{
    s_debug_level = new_level;
}


void pdebughex(uint8_t *src, uint16_t len){

}
void pdebug(const char *format, ...)
{
    int len = 0;
    uint8_t *ptr = debug_buf;

    if(s_debug_level >= DEBUG_LEVEL_DEBUG)
    {
        memset(debug_buf,0,sizeof(debug_buf));
        va_list ap;
        va_start(ap, format);
        vsnprintf((char *)debug_buf, LOG_SIZE - 1, format, ap);
        va_end(ap);

        len = strlen((char *)debug_buf);
        debugWrite(ptr,LOG_SIZE);
    }
}
void perr(const char *format, ...){}
void pinfo(const char *format, ...)
{
    int len = 0;
    uint8_t *ptr = debug_buf;

    if(s_debug_level >= DEBUG_LEVEL_INFO)
    {
        memset(debug_buf,0,sizeof(debug_buf));
        va_list ap;
        va_start(ap, format);
        vsnprintf((char *)debug_buf, LOG_SIZE - 1, format, ap);
        va_end(ap);

        len = strlen((char *)debug_buf);
        debugWrite(ptr,LOG_SIZE);
    }

}
void pprint(const char *format, ...){}
void phex(uint8_t *src, uint16_t len){}
void perrhex(uint8_t *src, uint16_t len){}

void log_print(const char *fmt, ...)
{
    int len = 0;

    uint8_t *ptr = debug_buf;
    memset(debug_buf,0,sizeof(debug_buf));
    va_list ap;
    va_start(ap, fmt);
    vsnprintf((char *)debug_buf, LOG_SIZE - 1, fmt, ap);
    va_end(ap);

    len = strlen((char *)debug_buf);
    debugWrite(ptr,LOG_SIZE);
}
