#ifndef _MYF_H_
#define _MYF_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"



typedef struct
{
  uint8_t  id[4];
  uint16_t imgWidth;
  uint16_t imgHeight;
  uint16_t clutOffset;
  uint16_t clutUsed;
  uint16_t reserved;
  uint16_t sequenceOffset;
  uint32_t sequenceSize;
}MYFHEAD_T;

typedef struct
{
  uint32_t firstPixIndx;
  uint32_t lastPixIndx;
  uint16_t nDraw;
  uint16_t nSkip;  
  uint16_t sc;
  uint16_t ec;
  uint16_t sp;
  uint16_t ep;
}DRAWBOUNDS_T;

typedef struct
{
  int32_t last;
  int32_t npix;
  int32_t start;
  int32_t end;
  int32_t rep;
  int32_t remrep;
  uint8_t colIndx;
  uint8_t tmp;
}CONTDRAW_T;

class CMYF
{
public:
  CMYF();
  ~CMYF();
  CLCD lcd;
  bool GetDrawBounds(int16_t x, int16_t y, uint16_t width, uint16_t height, DRAWBOUNDS_T* db);
  void Draw_MYF(uint8_t* myf, int16_t x, int16_t y);
  uint8_t* Draw_MYF_Start(uint8_t* buf, uint16_t bufsize, int16_t x, int16_t y);
  uint8_t* Draw_MYF_Continue(uint8_t* buf, uint16_t bufsize);
  void DrawPart(uint8_t* myf, DRAWBOUNDS_T* db);
  uint8_t* DrawPartFromRAM(uint8_t* buf, uint16_t bufsize, DRAWBOUNDS_T* db, CONTDRAW_T* cd);


private:
  uint32_t GetMyfInfo(void* filebuf);
  void InitContDraw(CONTDRAW_T* cd, DRAWBOUNDS_T* db);
  
public:
  MYFHEAD_T _head;
  uint16_t _clut[254];
  DRAWBOUNDS_T _db;
private:
  CONTDRAW_T _cd;
  
};

extern "C" void DrawPixelSequenceFull_Fast(uint8_t* seq, uint32_t seqSize, uint16_t* clut, uint32_t GPIOx_BASE);
void DrawPixelSequenceFull(uint8_t* seq, uint32_t seqSize, uint16_t* clut);


#endif /* _MYF_H_ */