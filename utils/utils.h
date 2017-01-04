#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include "stm32f0xx.h"

uint32_t Get_LE32(uint8_t* p32);
uint32_t Get_BE32(uint8_t* p32);


#endif /* _UTILS_H_ */