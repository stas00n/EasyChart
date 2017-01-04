#include "lcd.h"
#include "Timing.h"
  
CLCD::CLCD(){;}
CLCD::~CLCD(){;}

void CLCD::Init()
{
  GPIO_ResetPin(PIN_LCD_RST);
  GPIO_SetPin(PIN_LCD_RD);
  GPIO_SetPin(PIN_LCD_WR);
  GPIO_SetPin(PIN_LCD_CS);
  __delay(50000);
  GPIO_SetPin(PIN_LCD_RST);
  __delay(50000);
  
  WriteCom(1);      // SW Reset  
  WriteCom(0x11);   // Exit Sleep
  __delay(100000);
  
  WriteCom(0x36);   //Set Address Mode
  WriteData(0x48);  //(0x48);
  WriteCom(0x3A);   //Set Pixel Format
  WriteData(0x05);
  WriteCom(0x29);   //set display on
}

void CLCD::SetColumnAddress(uint16_t startCol, uint16_t endCol)
{
  WriteCom(0x2A);
  WriteData(startCol >> 8);
  WriteData((uint8_t)startCol);
  WriteData(endCol >> 8);
  WriteData((uint8_t)endCol);
}

void CLCD::SetPageAddress(uint16_t startPg, uint16_t endPg)
{
  WriteCom(0x2B);
  WriteData(startPg >> 8);
  WriteData((uint8_t)startPg);
  WriteData(endPg >> 8);
  WriteData((uint8_t)endPg);
}

void CLCD::FillRect(CRect* rect, uint16_t color)
{
  uint32_t nPixels = rect->width * rect->height;
  SetColumnAddress(rect->left, rect->left + rect->width - 1);
  SetPageAddress(rect->top, rect->top + rect->height - 1);
  WriteCom(0x2C);
  
  uint16_t pixel = color;
  
  WritePixels(pixel, nPixels/*, GPIOC_BASE*/);

}

void CLCD::MemRect(CRect* rect, uint16_t* mem)
{
  uint16_t pix;
  uint32_t nPixels = rect->width * rect->height;
  SetColumnAddress(rect->left, rect->left + rect->width - 1);
  SetPageAddress(rect->top, rect->top + rect->height - 1);
  WriteCom(0x2C);
  
    
  for(uint32_t i = 0; i < nPixels; i++)
  {
    pix = *mem++;
    WritePixels(pix, 1/*, GPIOC_BASE*/);
  }
}

void CLCD::DrawBitmap(CRect* rect, uint16_t* bm)
{
  uint32_t nPixels = rect->width * rect->height;
  SetColumnAddress(rect->left, rect->left + rect->width - 1);
  SetPageAddress(rect->top, rect->top + rect->height - 1);
  WriteCom(0x2C);

  WritePixelsBitmap2(bm, nPixels, GPIOC_BASE);
}

void CLCD::WriteCom(uint8_t com)
{
  GPIO_ResetPin(PIN_LCD_CS);
  GPIO_ResetPin(PIN_LCD_DCX);
  GPIO_ResetPin(PIN_LCD_WR);
  GPIOC->ODR = com;
  GPIO_SetPin(PIN_LCD_WR);
  GPIO_SetPin(PIN_LCD_DCX);
}

void CLCD::WriteData (uint8_t data)
{
  GPIO_ResetPin(PIN_LCD_CS);
  GPIO_SetPin(PIN_LCD_DCX);
  GPIO_ResetPin(PIN_LCD_WR);
  GPIOC->ODR = data;
  GPIO_SetPin(PIN_LCD_WR);
}

void CLCD::Clear(uint16_t color)
{
  CRect rect;
  rect.left = DISP_COL_MIN;
  rect.width = DISP_WIDTH;
  rect.top = DISP_PAGE_MIN;
  rect.height = DISP_HEIGHT;
  FillRect(&rect, color);
}