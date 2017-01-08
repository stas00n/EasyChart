#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>
#include <stdlib.h>
#include "stm32f0xx.h"
#include "hardware.h"
#include "gpioex.h"
#include "utils.h"
#include "Timing.h"
//#include "Fonts/DefaultFonts.h"
#include "Fonts/Fonts.h"

//  screen area desription
#define DISP_WIDTH      272
#define DISP_HEIGHT     480
#define DISP_COL_OFFSET 24
#define DISP_PAGE_OFFSET 0

#define DISP_COL_MIN    DISP_COL_OFFSET
#define DISP_COL_MAX    (DISP_COL_OFFSET + DISP_WIDTH - 1)
#define DISP_PAGE_MIN   DISP_PAGE_OFFSET
#define DISP_PAGE_MAX   (DISP_PAGE_OFFSET + DISP_HEIGHT - 1)

typedef struct
{
  uint8_t width;
  uint8_t height;
  uint8_t offset;
  uint8_t numchars;
}font_t;

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
  uint8_t       _width;
  uint8_t       _height;
  uint16_t      _trColor;
  int           _x;
  int           _y;
  uint16_t*     _bm;
  uint16_t*     _bmbkg;
  uint16_t*     _bmov;
  bool          _bkgCaptured;
  
public:
  CSprite();
  ~CSprite();
  bool Create(uint8_t width, uint8_t height, uint16_t* bm = NULL);
  void Destroy();
  void Overlay();
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
  
  int SetDrawRect(CRect* rect);
  void FillRect(CRect* rect, uint16_t pColor);
  void DrawBitmap(CRect* rect, uint16_t* bm);
  void ReadBitmap(CRect* rect, uint16_t* bm);
  void ReadPixels(uint16_t* buf, uint32_t nPixels);
  void PutChar(char c, int x, int y);
  void PutCharTransparent(char c, int x, int y);
  void Print(char* str, int x, int y, int* strkern = NULL);
  void DrawSprite(CSprite* sprite, int x, int y);
  void ClearSprite(CSprite* sprite);
  
public:
  FONT_INFO* _font;
  uint16_t   _bkCol;
  uint16_t   _penCol;
  bool       _trPrint;
};

/*---------------------"C" Linkage funcs:------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

void WritePixel(uint16_t* pPixel);  
void WritePixels(uint16_t pixel, uint32_t nPixels);
void WritePixelsBitmap(uint16_t* bm, uint32_t nPixels);
void WritePixelsBitmap2(uint16_t* bm, uint32_t nPixels);
void WriteComA(uint8_t com);
void WriteDataA(uint8_t com);


#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/


#endif /* _LCD_H_ */