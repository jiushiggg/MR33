#include <stdint.h>
#include "utils.h"
#include "sys_cfg.h"

uint16_t bitmap_count(uint8_t *data, uint16_t count)
{
	uint16_t n = 0;
    uint8_t packet_byte = (count+7) >> 3;
    uint8_t i;
    
	for (i = 0; i < packet_byte; i++)	{
		uint8_t a = data[i];
		while(a != 0) {
			n++;
			a = a & (a-1);
		}
	}
	
	return n;		
}

int8_t bitmap_check(uint8_t* bitmap, uint16_t idx)
{
    uint16_t packet_byte = idx / 8;
    uint8_t packet_bit = (uint8_t)(idx % 8);

    if(packet_byte > MAX_TRANS_PACKET_NUM)
        return BITMAP_OVERFLOW;

    if((bitmap[packet_byte] & (0x01<<packet_bit)) == 0)
    	return BITMAP_CLEARED;

    return BITMAP_UNCLEAR;
}

int8_t bitmap_clear_bit(uint8_t* bitmap, uint16_t idx)
{
    uint16_t packet_byte = idx / 8;
    uint8_t packet_bit = (uint8_t)(idx % 8);
    
    if(packet_byte > MAX_TRANS_PACKET_NUM)
        return BITMAP_OVERFLOW;
        
	bitmap[packet_byte] &= ~(0x01<<packet_bit);

	return BITMAP_CLEARED;
}

int8_t bitmap_set_bit(uint8_t* bitmap, uint16_t idx)
{
    uint16_t packet_byte = idx / 8;
    uint8_t packet_bit = (uint8_t)(idx % 8);

    if(packet_byte > MAX_TRANS_PACKET_NUM)
        return BITMAP_OVERFLOW;
    
	bitmap[packet_byte] |= (0x01<<packet_bit);

	return BITMAP_UNCLEAR;
}

int8_t bitmap_set(uint8_t* bitmap, uint16_t packet)
{
    uint16_t packet_byte = packet / 8;
    uint8_t packet_bit = (uint8_t)(packet % 8);
    uint8_t idx;

    if(packet_byte > MAX_TRANS_PACKET_NUM)
        return BITMAP_OVERFLOW;
    
    for(idx=0; idx<packet_byte; idx++)
        bitmap[idx] = 0xFF;

    bitmap[idx] = (1<<packet_bit)-1;    

	return packet;
}

uint16_t bitmap_get(uint8_t* bitmap, uint16_t idx, uint16_t count)
{
    uint16_t start_byte = idx / 8;
    uint8_t start_bit = (uint8_t)(idx % 8);
    uint16_t end_byte = (idx+count-1) / 8;
    uint8_t end_bit = (uint8_t)((idx+count-1) % 8);
    uint16_t bit_info = 0;

    if( count == 0 || count > 16
        || start_byte > MAX_TRANS_PACKET_NUM
        || end_byte > MAX_TRANS_PACKET_NUM )
        return 0;

    if(start_byte != end_byte) {  
        bit_info =  GET_BIT(bitmap[end_byte], 0, end_bit);
        if(start_byte+1 != end_byte) {
            bit_info <<= 8;
            bit_info |= bitmap[start_byte+1];
        }
        bit_info <<= (7-start_bit+1);
        bit_info |=  GET_BIT(bitmap[start_byte], start_bit, 7);
    } else {
        bit_info =  GET_BIT(bitmap[start_byte], start_bit, end_bit);  
    }

	return bit_info;
}

uint16_t bit_pos(uint16_t x)
{
    int r = 1;
    if (!x)
        return 0;
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }

    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }

    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }

    if (!(x & 1)) {
        x >>= 1;   
        r += 1;
    }

    return r;
}



