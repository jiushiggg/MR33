#ifndef COMMON_CRC16_H_
#define COMMON_CRC16_H_
#include <stdint.h>

uint16_t Crc16_Cal(const uint8_t* buf, uint32_t len);
uint16_t CRC16_CaculateStepByStep(uint16_t crc, const uint8_t* buf, uint32_t len);

#endif
