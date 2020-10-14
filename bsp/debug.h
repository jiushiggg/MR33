
#ifndef _DEBUG_H
#define _DEBUG_H
#include <stdint.h>
#include <stdarg.h>

#define DEBUG_LEVEL_HST			0
#define	DEBUG_LEVEL_ERROR		1	
#define	DEBUG_LEVEL_INFO		2	
#define	DEBUG_LEVEL_DEBUG		3

#define DEBUG_LEVEL_DFAULT		DEBUG_LEVEL_DEBUG
#define LOG_SIZE    32

typedef enum{
    DEBUG_SPI = (uint8_t)0,
    DEBUG_UART = (uint8_t)1,
}em_debug_peripheral;


uint8_t Debug_GetLevel(void);
void Debug_SetLevel(uint8_t new_level);
void pdebughex(uint8_t *src, uint16_t len);
void pdebug(const char *format, ...);
void perr(const char *format, ...);
void pinfo(const char *format, ...);
void pprint(const char *format, ...);
void phex(uint8_t *src, uint16_t len);
void perrhex(uint8_t *src, uint16_t len);
void debug_peripheral_init(void);
void log_print(const char *fmt, ...);
void pinfoEsl(const char *format, ...);

#define TRACE()   pinfo("trace: %s", __FUNCTION__)

#endif
