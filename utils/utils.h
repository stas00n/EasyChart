#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include "stm32f0xx.h"

uint32_t Get_LE32(uint8_t* p32);
uint32_t Get_BE32(uint8_t* p32);
void _utoa( int value, char * string);
void memset16(void* memptr, uint16_t val, uint32_t num);

#endif /* _UTILS_H_ */