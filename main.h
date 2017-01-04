#ifndef _MAIN_H_
#define _MAIN_H_



#include "stm32f0xx_conf.h"
#include "lcd.h"
#include "hardware.h"
#include "periph.h"
#include "Timing.h"
#include "gpioex.h"
#include "myf.h"
#include "buttons.h"


void LoadFromFile(char* filename, int x, int y);


#endif /* _MAIN_H_ */