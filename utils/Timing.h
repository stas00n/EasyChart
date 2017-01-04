#ifndef _TIMING_H_
#define _TIMING_H_

#include <stdint.h>
#include "stm32f0xx.h"

#ifdef __cplusplus
extern "C" {
#endif

int GetTimeoutEvent();
int GetTimeoutTimer();
void SetTimeoutTimer(int timer_us);
void __delay(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* _TIMING_H_ */