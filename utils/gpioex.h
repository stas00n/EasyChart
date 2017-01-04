#ifndef _GPIOEX_H_
#define _GPIOEX_H_
  
#include "stm32f0xx.h"

  void GPIO_Init_Single(GPIO_InitTypeDef* gpio, GPIO_TypeDef* GPIOx, uint8_t pin, bool outLevel = 0, uint8_t af = GPIO_AF_0);

  inline void GPIO_SetPin(GPIO_TypeDef* port, uint16_t pin){port->BSRR = (1 << pin);}
  inline void GPIO_ResetPin(GPIO_TypeDef* port, uint16_t pin){port->BRR = (1 << pin);}
  
  uint16_t GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t pin);
  
#endif /* _GPIOEX_H_ */