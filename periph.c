#include "periph.h"

//*****************************************************************************

void GPIO_Config()
{
  GPIO_InitTypeDef gpio;
  gpio.GPIO_Mode = GPIO_Mode_OUT;
  gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio.GPIO_Speed = GPIO_Speed_Level_3;
  gpio.GPIO_Pin = 0xFF; // GPIOx0...GPIOx7
  
  GPIO_Init(GPIOLCD, &gpio);
  
  GPIO_Init_Single(&gpio, PIN_LCD_WR);
  GPIO_Init_Single(&gpio, PIN_LCD_RD);
  GPIO_Init_Single(&gpio, PIN_LCD_CS);
  GPIO_Init_Single(&gpio, PIN_LCD_RST);
  GPIO_Init_Single(&gpio, PIN_LCD_DCX);
  
  GPIO_Init_Single(&gpio, PIN_SPI_CS, 1);
  
  gpio.GPIO_Mode = GPIO_Mode_IN;
  gpio.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init_Single(&gpio, PIN_BTN);

  gpio.GPIO_Mode = GPIO_Mode_AF;
  gpio.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init_Single(&gpio, PIN_SPI_MOSI);
  GPIO_Init_Single(&gpio, PIN_SPI_MISO);
  GPIO_Init_Single(&gpio, PIN_SPI_SCK);
}

//*****************************************************************************

void Clock_Config()
{
  RCC_PLLCmd(DISABLE);
  RCC_PLLConfig(RCC_PLLSource_HSI, RCC_PLLMul_6);
  RCC_PLLCmd(ENABLE);
  while(!(RCC->CR & RCC_FLAG_PLLRDY)){;}
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA |
                        RCC_AHBPeriph_GPIOB |
                        RCC_AHBPeriph_GPIOC, ENABLE);
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
}