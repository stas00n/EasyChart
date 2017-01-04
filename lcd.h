#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>
#include <stdlib.h>
#include "stm32f0xx.h"
#include "hardware.h"
#include "gpioex.h"


//  screen area desription
#define DISP_WIDTH      272
#define DISP_HEIGHT     480
#define DISP_COL_OFFSET 24
#define DISP_PAGE_OFFSET 0

#define DISP_COL_MIN    DISP_COL_OFFSET
#define DISP_COL_MAX    (DISP_COL_OFFSET + DISP_WIDTH - 1)
#define DISP_PAGE_MIN   DISP_PAGE_OFFSET
#define DISP_PAGE_MAX   (DISP_PAGE_OFFSET + DISP_HEIGHT - 1)

class CRect
{
public:
  uint16_t top;
  uint16_t left;
  uint16_t width;
  uint16_t height;
};

class CSprite
{
public:
  
  uint8_t _width;
  uint8_t _height;

  uint16_t* _bm;
  uint16_t _trColor;
  int _x;
  int _y;
  
//private:
    uint16_t* _bmbkg;
    bool      _bkgCaptured;
    
public:
  CSprite();
  ~CSprite();
  bool Create(uint8_t width, uint8_t height, uint16_t* bm = NULL);
  
};

class CLCD
{
public:
  CLCD();
  ~CLCD();
  
  void WriteCom(uint8_t com);
  void WriteData (uint8_t data);
  
  void Init();
  void SetColumnAddress(uint16_t startCol, uint16_t endCol);
  void SetPageAddress(uint16_t startPg, uint16_t endPg);
  void Clear(uint16_t color = 0);
  
  inline void WriteMemoryStart(){WriteCom(0x2C);}
  inline void WriteMemoryContinue(){WriteCom(0x3C);}
  inline void ReadMemoryStart(){WriteCom(0x2E);}
  inline void ReadMemoryContinue(){WriteCom(0x3E);}
  void FillRect(CRect* rect, uint16_t pColor);
  void MemRect(CRect* rect, uint16_t* mem);
  void DrawBitmap(CRect* rect, uint16_t* bm);
  void ReadBitmap(CRect* rect, uint16_t* bm);
  
  void DrawSprite(CSprite* sprite, int x, int y);
  void ClearSprite(CSprite* sprite);

};

/*---------------------"C" Linkage funcs:------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

void WritePixel(uint16_t* pPixel);  
void WritePixels(uint16_t pixel, uint32_t nPixels/*, uint32_t GPIOx_BASE*/);
void WritePixelsBitmap(uint16_t* bm, uint32_t nPixels, uint32_t GPIOx_BASE);
void WritePixelsBitmap2(uint16_t* bm, uint32_t nPixels, uint32_t GPIOx_BASE);
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/


#endif /* _LCD_H_ */