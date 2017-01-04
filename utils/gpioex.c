#include "gpioex.h"

void GPIO_Init_Single(GPIO_InitTypeDef* gpio, GPIO_TypeDef* GPIOx, uint8_t pin, bool outLevel, uint8_t af)
{
  uint16_t GPIO_Pin = (1 << pin);
  // Set output level
  if(outLevel)
    GPIOx->BSRR = GPIO_Pin;
  else
    GPIOx->BRR = GPIO_Pin;
  // Select alt function
  if(gpio->GPIO_Mode == GPIO_Mode_AF)
    GPIO_PinAFConfig(GPIOx, (uint16_t)pin, af);
  // Set other parameters
  gpio->GPIO_Pin = GPIO_Pin;
  GPIO_Init(GPIOx, gpio);
}

//void GPIO_SetPin(GPIO_TypeDef* port, uint16_t pin){port->BSRR = (1 << pin);}

//void GPIO_ResetPin(GPIO_TypeDef* port, uint16_t pin){port->BRR = (1 << pin);}

uint16_t GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t pin)
{
  return (GPIOx->IDR & (1 << pin));
}