#include "buttons.h"



void Buttons_Init()
{
  GPIO_InitTypeDef gpio;
  gpio.GPIO_Mode = GPIO_Mode_IN;
  gpio.GPIO_OType = GPIO_OType_OD;
  gpio.GPIO_PuPd = GPIO_PuPd_UP;
  gpio.GPIO_Speed = GPIO_Speed_Level_1;
  
  GPIO_Init_Single(&gpio, PIN_BUTTON_1);
  GPIO_Init_Single(&gpio, PIN_BUTTON_2);
  GPIO_Init_Single(&gpio, PIN_BUTTON_3);
  GPIO_Init_Single(&gpio, PIN_BUTTON_4);
  
 // RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
 // SYSCFG->EXTICR[0] = 0;//EXTI lines connected to GPIOA  

  EXTI_InitTypeDef exti;
  exti.EXTI_Line = EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3;
  exti.EXTI_Mode = EXTI_Mode_Interrupt;
  exti.EXTI_Trigger = EXTI_Trigger_Falling;
  exti.EXTI_LineCmd = ENABLE;
  EXTI_Init(&exti);
  
  
  
#ifdef BUTTON_INTERRUPTS  
  NVIC_InitTypeDef nvic;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  nvic.NVIC_IRQChannel = EXTI0_1_IRQn;
  nvic.NVIC_IRQChannelPriority = 3;
  NVIC_Init(&nvic);
  nvic.NVIC_IRQChannel = EXTI2_3_IRQn;
  NVIC_Init(&nvic);
#endif /* BUTTON_INTERRUPTS */
}


uint32_t GetButton()
{
  uint32_t mask = 1;
  if(mask & EXTI->PR) return 1;
  mask <<= 1;
  if(mask & EXTI->PR) return 2;
  mask <<= 1;
  if(mask & EXTI->PR) return 3;
  mask <<= 1;
  if(mask & EXTI->PR) return 4;
  return 0;
}

void Clear_Buttons()
{
  EXTI->PR = 0xFFFF;
}