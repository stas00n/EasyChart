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
#include "Projection.h"



// Application defines
#define MAX_ZOOM        18
#define MIN_ZOOM        3

void LoadFromFile(char* filename, int x, int y);
void zoomIn(uint8_t* zoom);
void zoomOut(uint8_t* zoom);
void scrlH(PIXELPOINT_T* pt, signed char dx); 
void scrlV(PIXELPOINT_T* pt, signed char dy);
#endif /* _MAIN_H_ */