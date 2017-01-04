/*
 * MCU Peripherals init
*/


#ifndef _PERIPH_H_
#define _PERIPH_H_

#include "stm32f0xx.h"
#include "gpioex.h"
#include "hardware.h"


void GPIO_Config();
void Clock_Config();

#endif /* _PERIPH_H_ */