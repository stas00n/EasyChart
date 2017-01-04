#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include "stm32f0xx_conf.h"
#include "gpioex.h"


#define PIN_BUTTON_1 GPIOA, 0
#define PIN_BUTTON_2 GPIOA, 1
#define PIN_BUTTON_3 GPIOA, 2
#define PIN_BUTTON_4 GPIOA, 3

void Buttons_Init();
uint32_t GetButton();
void Clear_Buttons();

#endif /* _BUTTONS_H_ */