#include "Timing.h"

 int GetTimeoutTimer()
{
  return SysTick->VAL;
}

int GetTimeoutEvent()
{
  return (int)(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk);
}
 
void SetTimeoutTimer(int timer_us)
 {
   SysTick->CTRL &= !(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_COUNTFLAG_Msk);
   SysTick->LOAD = timer_us;
   SysTick->VAL = 0;
   SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
 }

void __delay(uint32_t us)
{
  SetTimeoutTimer(us);
  while(!GetTimeoutEvent()){;}
}