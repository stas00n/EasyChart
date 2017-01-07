#include "lcd.h"

  
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

int CLCD::SetDrawRect(CRect* rect)
{
  // TODO: Validate boundaries
  //int ret;
  int s, e;
  s = rect->left + DISP_COL_OFFSET;
  e = s + rect->width - 1;
  SetColumnAddress(s, e);
  s = rect->top + DISP_PAGE_OFFSET;
  e = s + rect->height - 1;
  SetPageAddress(s, e);
  return 1;//ret;
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
  WriteMemoryStart();
  
    
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
  WriteMemoryStart();

  WritePixelsBitmap2(bm, nPixels, GPIOC_BASE);
}

void CLCD::ReadBitmap(CRect* rect, uint16_t* bm)
{
  uint32_t nPixels = rect->width * rect->height;
  SetColumnAddress(rect->left, rect->left + rect->width - 1);
  SetPageAddress(rect->top, rect->top + rect->height - 1);
  ReadMemoryStart();

  GPIOLCD->MODER &= 0xFFFF0000;
  // Dummy read
  GPIO_ResetPin(PIN_LCD_RD);
  GPIO_SetPin(PIN_LCD_RD);
  
  uint8_t b;
  uint16_t w;
  for(uint32_t i = 0; i < nPixels; i++)
  {
    GPIO_ResetPin(PIN_LCD_RD);
    GPIO_SetPin(PIN_LCD_RD);
    b = (uint8_t)(GPIOLCD->IDR);
    b >>= 3;
    w = b << 11;
    
    GPIO_ResetPin(PIN_LCD_RD);
    GPIO_SetPin(PIN_LCD_RD);
    b = (uint8_t)(GPIOLCD->IDR);
    b >>= 2;
    w |= b << 5;
    
    
    GPIO_ResetPin(PIN_LCD_RD);
    GPIO_SetPin(PIN_LCD_RD);
    b = (uint8_t)(GPIOLCD->IDR);
    b >>= 3;
    w |= b;
    *bm = w;
    
    bm++;
  }
  GPIOLCD->MODER |= 0x5555;
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

CSprite::CSprite()
  {
    _bmbkg = _bm = NULL;
    _bkgCaptured = false;
  }
CSprite::~CSprite()
  {
    if(_bmbkg != NULL)
      free(_bmbkg);
    if(_bmov != NULL)
      free(_bmov);
  }
bool CSprite::Create(uint8_t width, uint8_t height, uint16_t* bm)
{
  _bmbkg = (uint16_t*)malloc(width * height * 2);
  if(_bmbkg == NULL)
    return false;
  _bmov = (uint16_t*)malloc(width * height * 2);
  if(_bmov == NULL)
    return false;
  
  _width = width;
  _height = height;
  _bkgCaptured = false;
  _bm = bm;
  return true;
}

void CSprite::Destroy()
{
  if(_bmbkg != NULL)
    free(_bmbkg);
  if(_bmov != NULL)
    free(_bmov);
  _bmbkg = NULL;
  _bmov = NULL;
  _width = 0;
  _height = 0;
}

void CSprite::Overlay()
{
  uint32_t n = _width * _height;
  uint16_t trcol = _trColor;
  while(n--)
  {
    if(_bm[n] != trcol)
      _bmov[n] = _bm[n];
    else
      _bmov[n] = _bmbkg[n];
  }
}

void CLCD::DrawSprite(CSprite* sprite, int x, int y)
{
  CRect r;
  r.width = sprite->_width;
  r.height = sprite->_height;
  
  if(sprite->_bkgCaptured)
  {
    r.left = sprite->_x;
    r.top = sprite->_y;
    MemRect(&r, sprite->_bmbkg);
  }
  else
  {
    sprite->_x = r.left = x;
    sprite->_y = r.top = y;
    ReadBitmap(&r, sprite->_bmbkg);
    sprite->_bkgCaptured = true;
  }
  
  if(sprite->_bm)
  {
    sprite->_x = r.left = x;
    sprite->_y = r.top = y;
    ReadBitmap(&r, sprite->_bmbkg);
    sprite->Overlay();
    MemRect(&r, sprite->_bmov);
  }
}

void CLCD::ClearSprite(CSprite* sprite)
{
  CRect r;
  r.width = sprite->_width;
  r.height = sprite->_height;
  
  if(sprite->_bkgCaptured)
  {
    r.left = sprite->_x;
    r.top = sprite->_y;
    MemRect(&r, sprite->_bmbkg);
  }
  sprite->_bkgCaptured = false;
}

void CLCD::PutChar(char c, int x, int y)
{
  if(c < _font->startCh || c > _font->endCh)
    return;
  c -= _font->startCh;
  
  CRect r;
  r.left = x;
  r.top = y;
  r.width = _font->desc[c].chWidth + _font->interval;
  r.height = _font->height;
  SetDrawRect(&r);
  
  uint16_t w = _font->desc[c].chWidth;
  uint8_t t;
  uint8_t* pchbitmap;
  uint16_t pixels[MAX_FONT_WIDTH];
  WriteMemoryStart();
  
  pchbitmap = (uint8_t*)_font->bitmaps + _font->desc[c].chOffset;
  for(uint32_t i = 0; i < r.height; i++)
  {
    memset16(pixels, _bkCol, r.width);
    
    for(uint8_t j = 0; j < w; j++)
    {
      t = *(pchbitmap + i * (1 + ((w - 1) >> 3)) + (j >> 3));
      t <<= (j & 7);
      if(t & 0x80) pixels[j] = _penCol;
    }

    WritePixelsBitmap(pixels, r.width);
  }
}



void CLCD::PutCharTransparent(char c, int x, int y)
{
  if(c < _font->startCh || c > _font->endCh)
    return;
  c -= _font->startCh;
  
  CRect r;
  r.left = x;
  r.top = y;
  r.width = _font->desc[c].chWidth + _font->interval;
  r.height = _font->height;
  SetDrawRect(&r);
  
  uint16_t w = _font->desc[c].chWidth;
  uint8_t t;
  uint8_t* pchbitmap;
  uint16_t pixels[MAX_FONT_WIDTH];

  pchbitmap = (uint8_t*)_font->bitmaps + _font->desc[c].chOffset;
  for(uint32_t i = 0; i < r.height; i++)
  {
    ReadMemoryStart();
    ReadPixels(pixels, r.width);
    for(uint8_t j = 0; j < w; j++)
    {
      t = *(pchbitmap + i * (1 + ((w - 1) >> 3)) + (j >> 3));
      t <<= (j & 7);
      if(t & 0x80) pixels[j] = _penCol;
    }
    WriteMemoryStart();
    WritePixelsBitmap(pixels, r.width);
    //r.top++;
    //SetDrawRect(&r);
    SetPageAddress(r.top + i + 1, r.top + r.height); 
  }
}

void CLCD::Print(char* str, int x, int y, int* strkern)
{
  int X = x;
  int pos = -1;
  while (char c = *(str++))
  {
    pos++;
    // Detect <CR>
    if(c == '\r') 
    {
      X = x;
      continue;
    }
    
    // Detect <LF>
    if(c == '\n')
    {
      y += _font->height;
      continue;
    }
    
    //    if(c == ' ')
    //    {
    //      X += _font->spaceW;
    //      continue;
    //    }
    
    if(c < _font->startCh || c > _font->endCh)
      c = ' ';
    
    // Auto <CR><LF>
    if(X + _font->desc[c - _font->startCh].chWidth > DISP_WIDTH)
    {
      X = x;
      y += _font->height;
    }
    
    //PutChar(c, X, y);
    PutCharTransparent(c, X, y);
    c -= _font->startCh;
    X += _font->desc[c].chWidth + _font->interval;
    if(strkern)
      X += strkern[pos];
  }
}

void CLCD::ReadPixels(uint16_t* buf, uint32_t nPixels)
{
  GPIOLCD->MODER &= 0xFFFF0000;
  // Dummy read
  GPIO_ResetPin(PIN_LCD_RD);
  GPIO_SetPin(PIN_LCD_RD);
    
  uint8_t b;
  uint16_t w;
  for(uint32_t i = 0; i < nPixels; i++)
  {
    GPIO_ResetPin(PIN_LCD_RD);
    GPIO_SetPin(PIN_LCD_RD);
    b = (uint8_t)(GPIOLCD->IDR);
    b >>= 3;
    w = b << 11;
    
    GPIO_ResetPin(PIN_LCD_RD);
    GPIO_SetPin(PIN_LCD_RD);
    b = (uint8_t)(GPIOLCD->IDR);
    b >>= 2;
    w |= b << 5;
    
    
    GPIO_ResetPin(PIN_LCD_RD);
    GPIO_SetPin(PIN_LCD_RD);
    b = (uint8_t)(GPIOLCD->IDR);
    b >>= 3;
    w |= b;
    
    *(buf++) = w;
  }
  GPIOLCD->MODER |= 0x5555;
}