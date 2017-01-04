#include "myf.h"

CMYF::CMYF()
{

}

CMYF::~CMYF()
{

}


bool CMYF::GetDrawBounds(int16_t x, int16_t y, uint16_t width, uint16_t height, DRAWBOUNDS_T* db)
{
  int sc;
  bool fits = true;
  // x bounds
  if(x < 0)
  {
    sc = DISP_COL_MIN;
    fits = false;
  }
  else sc = DISP_COL_MIN + x;
  
  if(sc > DISP_COL_MAX)
  {
    sc = DISP_COL_MAX;
    fits = false;
  }
  db->sc = (uint16_t)sc;
  
  int ec;
  ec = DISP_COL_MIN + x + width - 1;
  if(ec > DISP_COL_MAX)
  {
    ec = DISP_COL_MAX;
    fits = false;
  }
  if(ec < sc)
    ec = sc;
  
  if(ec > DISP_COL_MAX)
  {
    ec = DISP_COL_MAX;
    fits = false;
  }
  
  db->ec = (uint16_t)ec;
  
  // y bounds
  int sp;
  if(y < 0)
  {
    sp = DISP_PAGE_MIN;
    fits = false;
  } 
  else sp = DISP_PAGE_MIN + y;
  
  if(sp > DISP_PAGE_MAX)
  {
    sp = DISP_PAGE_MAX;
    fits = false;
  }
  db->sp = (uint16_t)sp;
  
  int ep;
  ep = DISP_PAGE_MIN + y + height - 1;
  if(ep < sp)
    ep = sp;
  if(ep > DISP_PAGE_MAX)
  {
    ep = DISP_PAGE_MAX;
    fits = false;
  }
  db->ep = (uint16_t)ep;
  
  // First
  if(y < 0) db->firstPixIndx = -y * width;
  else db->firstPixIndx = 0;
  if(x < 0) db->firstPixIndx += -x;
  // Last
  db->lastPixIndx = db->firstPixIndx + (ep - sp) * width + ec - sc;
  // Draw-Skip
  db->nDraw = ec - sc + 1;
  db->nSkip = width - (ec - sc) - 1;
  
  return fits;
}

void CMYF::Draw_MYF(uint8_t* myf, int16_t x, int16_t y)
{
  //GetDrawBounds(DRAWBOUNDS_T* db
}



uint8_t* CMYF::Draw_MYF_Start(uint8_t* buf, uint16_t bufsize, int16_t x, int16_t y)
{
  if(!GetMyfInfo(buf))
    return 0;
  
  bool fits = GetDrawBounds(x, y, _head.imgWidth, _head.imgHeight,&_db);
  InitContDraw(&_cd, &_db);
  
  lcd.SetColumnAddress(_db.sc, _db.ec);
  lcd.SetPageAddress(_db.sp, _db.ep);
  
  lcd.WriteMemoryStart();
  
  buf += _head.sequenceOffset; 

  return DrawPartFromRAM(buf, bufsize - _head.sequenceOffset, &_db, &_cd);
}

uint8_t* CMYF::Draw_MYF_Continue(uint8_t* buf, uint16_t bufsize)
{
  lcd.WriteMemoryContinue();
  return DrawPartFromRAM(buf, bufsize, &_db, &_cd);
}

void DrawPixelSequenceFull(uint8_t* seq, uint32_t seqSize, uint16_t* clut)
{
  uint8_t colIndx, tmp;
  uint16_t rep;
  
  while(seqSize--)
  {
    tmp = *seq++;
    if(tmp < 0xFE)
    {
      colIndx = tmp;
      WritePixels(clut[tmp], 1/*, GPIOC_BASE*/);
    }
    else
    {
      rep = *(seq++);
      seqSize--;
      if(tmp == 0xFE)
      {
        rep |= (*(seq++) << 8);
        seqSize--;
      }
      WritePixels(clut[colIndx], rep/*, GPIOC_BASE*/);
    }
  }
} 

void CMYF::DrawPart(uint8_t* myf, DRAWBOUNDS_T* db)
{
  MYFHEAD_T* head = (MYFHEAD_T*)myf;
  uint16_t* clut = (uint16_t*)(myf + head->clutOffset);
  uint8_t* seq = myf + head->sequenceOffset;

  int32_t last = db->lastPixIndx;
  int32_t npix = 0;
  int32_t start = db->firstPixIndx;
  int32_t end = start + db->nDraw - 1;
  uint32_t wrote = 0;// debug
  lcd.WriteCom(0x2C);
  
  register uint8_t tmp;
  uint8_t colIndx;
  int32_t rep;
  int32_t remrep = 0;
  
  //bool foundstart = false;
  
  while(npix < last)
  {
    if(npix > end)
    {
      start += head->imgWidth;
      end += head->imgWidth;
    }
    // process remaining repeats
    if(remrep > 0)
    {
      if(npix < start || npix > end)//Don't draw
      {
        npix++;
        remrep--;
        continue; 
      }
      if(npix + remrep > end)//Draw to end
      {
        rep = end - npix + 1;
        WritePixels(clut[colIndx], rep/*, GPIOC_BASE*/);
        wrote += rep;
        npix += rep;
        remrep -= rep;
        continue;
      }
      WritePixels(clut[colIndx], remrep/*, GPIOC_BASE*/);// Draw on visible area
      wrote += remrep;
      npix += remrep;
      remrep = 0;
      continue;
    }
    //***********************************************************
    //read data    
    tmp = *seq++;
    
    if(tmp >= 0xFE)
    {
      remrep = *seq++; // Read repeats
      if(tmp == 0xFE)
        remrep |= ((*seq++) << 8);
      
      continue;
    }
    
    // single pixel write
    else
    {
      colIndx = tmp;
      if(npix < start || npix > end)
      {
        npix++;
        continue;
      }
      WritePixels(clut[tmp], 1/*, GPIOC_BASE*/);
      wrote += 1;
      npix++;
    }
  }
}


//*********************************************************************
uint8_t* CMYF::DrawPartFromRAM(uint8_t* buf, uint16_t bufsize, DRAWBOUNDS_T* db, CONTDRAW_T* cd)
{
  
  uint16_t* clut = _clut;
  uint8_t* seq = buf;
  uint8_t* max = seq + bufsize;
   int32_t last = cd->last;
  int32_t npix = cd->npix;
  int32_t start = cd->start;
  int32_t end = cd->end;
  uint8_t colIndx = cd->colIndx;
  int32_t rep = cd->rep;
  int32_t remrep = cd->remrep;
  

  register uint8_t tmp = cd->tmp;
  if(tmp > 0xfe)
    goto _enter;
  
  while(npix < last)
  {
    if(npix > end)
    {
      start += _head.imgWidth;
      end += _head.imgWidth;
    }
    // process remaining repeats
    if(remrep > 0)
    {
      if(npix < start || npix > end)//Don't draw
      {
        npix++;
        remrep--;
        continue; 
      }
      if(npix + remrep > end)//Draw to end
      {
        rep = end - npix + 1;
        WritePixels(clut[colIndx], rep/*, GPIOC_BASE*/);
        npix += rep;
        remrep -= rep;
        continue;
      }
      WritePixels(clut[colIndx], remrep/*, GPIOC_BASE*/);// Draw on visible area
      npix += remrep;
      remrep = 0;
      continue;
    }
    //***********************************************************
    

    
    //read data 
    tmp = *seq++;
    
    if(seq > max/*buf + bufsize*/)//Save contdraw and exit
    {
_save_quit:
      cd->colIndx = colIndx;
      cd->end = end;
 //     cd->last = last;
      cd->npix = npix;
      cd->remrep = remrep;
      cd->rep = rep;
      cd->start = start;
      cd->tmp = tmp;
      return seq;
    }

   
    if(tmp >= 0xFE)
    {
_enter: 
      if(seq < max/*buf + bufsize*/)
        remrep = *seq++; // Read repeats
      else 
        goto _save_quit;
      if(tmp == 0xFE)
        remrep |= ((*seq++) << 8);
      continue;
    }
    
    // single pixel write
    else
    {
      colIndx = tmp;
      if(npix > end || npix++ < start)
      {
        continue;
      }
      WritePixel(clut + tmp);
    }
  }
  return 0;
}

//void CMYF::mem_cpy(uint8_t* dest, uint8_t* src, size_t num)
//{
//  for(uint32_t i = 0; i < num; i++)
//  {
//    *(dest++) = *(src++);
//  }
//}

uint32_t CMYF::GetMyfInfo(void* filebuf)
{
  uint32_t* pid = (uint32_t*) filebuf;
  if(*pid != 0x4d46594d)
    return 0;
  
  MYFHEAD_T* head = (MYFHEAD_T*) filebuf;
  
  // TODO: non-safe copy, to be revised.
  memcpy((void*)&_head, filebuf, 20);
  memcpy(_clut, (uint8_t*)filebuf + head->clutOffset, head->clutUsed * 2);
  return 1;
}

void CMYF::InitContDraw(CONTDRAW_T* cd, DRAWBOUNDS_T* db)
{
  cd->last = db->lastPixIndx;
  cd->npix = 0;
  cd->start = db->firstPixIndx;
  cd->end = cd->start + db->nDraw - 1;
 // cd->colIndx = 0;
 // cd->rep = 0;
  cd->remrep = 0;
  cd->tmp = 0;
}