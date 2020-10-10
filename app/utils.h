#ifndef _UTILS_H_
#define _UTILS_H_

#include "bsp_spi.h"

#define BITMAP_CLEARED	0
#define BITMAP_UNCLEAR	1
#define BITMAP_OVERFLOW	-1

/*max number in two numbers*/
#if !defined(MAX)
#define MAX(a, b) ((a)>(b)?(a):(b))
#endif
/*min number in two numbers*/
#if !defined(MIN)
#define MIN(a, b) ((a)<(b)?(a):(b))
#endif

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

//! \brief macro to convert from SYS BIOS Ticks to ms
#define CC2640RF2_tick_To_ms(tick) ((tick) / (4000000/1000))

#define CC2640RF2_ms_To_tick(ms) ((ms)*1000/Clock_tickPeriod)

#define CC2640RF2_us_To_tick(us) ((us)/Clock_tickPeriod)

//杩欓噷灏辨槸瑕佹妸鏁板瓧x鐨勭n浣�(bitn浣�)缃负1
//1U灏辫〃绀虹殑鏄棤绗﹀彿鐨�1锛屽畯瀹氫箟鍙互浼犲弬鐨�
#define SET_BIT(x,n)    (x | 1U<<(n))

//杩欓噷灏辨槸瑕佹妸鏁板瓧x鐨勭n浣�(bitn浣�)娓呴浂
#define CLEAR_BIT(x,n)    (x & ~(1U<<(n)))

//杩欓噷灏辨槸瑕佹妸鏁板瓧x鐨勭n鍒癿浣嶇疆涓�1(n鏄綆浣嶏紝m鏄珮浣�)
//灏辨槸鍏堟妸0鍙栧弽灏卞彲浠ュ緱鍒板緢澶氱殑1锛岀劧鍚庡乏绉诲氨寰楀埌閭ｄ箞澶氫釜0锛屽啀鍙栧弽灏卞彲浠ュ緱鍒颁綘鎯宠鐨�1鐨勪釜鏁颁簡
//鏈�鍚庡乏绉讳綅鎴栧氨鍙互缃�1浜�
#define SET_BITS(x,n,m)    (x | ~(~0U<<(m-n+1))<<(n))

//鎴彇鍙橀噺鐨勯儴鍒嗚繛缁綅銆�(灏辨槸鍙栧嚭鐨勬剰鎬�)
//鍏跺疄鍜屼笂闈㈤偅閲屾槸宸笉澶氱殑锛屽悗闈㈤偅涓�澶ч儴鍒嗛兘鏄负浜嗙‘瀹氶渶瑕佸灏戜釜1
//鏈�鍚庤寰楀彸绉伙紝涓轰簡寰楀嚭閭ｄ釜鏁板瓧
#define GET_BIT(x,n,m)    (x & ~(~0U<<(m-n+1))<<(n)) >>(n)

uint16_t bitmap_count(uint8_t *data, uint16_t count);
int8_t bitmap_check(uint8_t* bitmap, uint16_t idx);
int8_t bitmap_clear_bit(uint8_t* bitmap, uint16_t idx);
int8_t bitmap_set_bit(uint8_t* bitmap, uint16_t idx);
int8_t bitmap_set(uint8_t* bitmap, uint16_t packet);
uint16_t bitmap_get(uint8_t* bitmap, uint16_t idx, uint16_t count);
uint16_t bit_pos(uint16_t x);

void wait_ms(uint32_t ms);

#endif
