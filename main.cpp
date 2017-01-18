#include "main.h"

CLCD lcd;
CMYF myf;

ASYNCIO_T asyncio;

void main()
{
  Clock_Config();
  GPIO_Config();
  SD_SPI_Driver_Init();
  
  Buttons_Init();
  lcd.Init();
  lcd.Clear();
  lcd._bkCol = 0;
  lcd._font = (FONT_INFO*)&arialNarrow_10ptFontInfo;
  lcd._penCol = 0xffff;
  struct
  {
    int x;
    int y;
  }con;
  con.x = 0; con.y = 0;
  FRESULT fresult;
  FATFS fs;
  uint8_t retry = 10;
  while(retry--)
  {
    fresult = f_mount(&fs, "0", 1);
    if(fresult == FR_OK) break;
  }
  if(fresult != FR_OK)
    lcd.Print("Error: Can't init SD Card!", con.x, con.y);
  else
    lcd.Print("SD card init OK.", con.x, con.y);
  
    con.y += lcd._font->height;

  // Allocate double buffer for async operation
  uint8_t* dblbuf[2];
<<<<<<< HEAD
  const int bufsize = 1100;
=======
  const int bufsize = 550;
>>>>>>> f51ce0c6bbbbced61c2b65e75b14fe06774b20e6
  dblbuf[0] = (uint8_t*)malloc(bufsize);
  dblbuf[1] = (uint8_t*)malloc(bufsize);
#ifdef DEBUG
  if(dblbuf[0] == NULL || dblbuf[1] == NULL)
  {
    lcd.Print("Error: Not enough memory for buffer.", con.x, con.y);
    con.y += lcd._font->height;
    while(1){;}
  }
  // Unaligned memcpy() test. Working in IAR, other compilers not tested
  uint8_t* d_e = (uint8_t*)0x20001f9c;  // even dest
  uint8_t* d_o = (uint8_t*)0x20001f9b;  // odd dest
  uint8_t* se = (uint8_t*)0x08001f9c;   // even src
  uint8_t* so = (uint8_t*)0x08001f9b;   // odd src
  memcpy(d_e, so, 80);
  memcpy(d_o, so, 80);
  memcpy(d_e, se, 80);
  memcpy(d_o, se, 80);
  lcd.Print("memcpy good.", con.x, con.y);// Test ok, if no HardFault here :)
  con.y += lcd._font->height;
#endif // DEBUG
  
//  char filepath[MAX_PATH];

//  CRect rect;
//  rect.left = 60;
//  rect.width = 200;
//  rect.top = 40;
//  rect.height = 1;
  
//  lcd.Clear();
  FIL f;
  while(1)
  {
    
    fresult = f_open(&f, "2.myf", FA_READ | FA_OPEN_EXISTING);
    if(fresult != FR_OK) continue;
    UINT br = 1;//заглушка пока что

    uint8_t sw;                // Buffers switch
    sw = 0;
    
    f_aread(&f, dblbuf[sw], bufsize, /*&br,*/ &asyncio);
    while(!asyncio.readComplete);
    f_aread(&f, dblbuf[sw ^ 1], bufsize,/* &br,*/ &asyncio);
    
    uint8_t* pos = myf.Draw_MYF_Start(dblbuf[sw], bufsize, 0, 0);

    while(br)
    {
      sw++;                     // Toggle buffers
      sw &= 1;
      
      while(!asyncio.readComplete);
      f_aread(&f, dblbuf[sw ^ 1], bufsize,/* &br,*/ &asyncio);
      pos = myf.Draw_MYF_Continue(dblbuf[sw], bufsize);
    }
    f_close(&f);
    lcd.Clear();
    
  }
}

//void LoadFromFile(char* filename, int x, int y)
//{
//  int n = 1; 
//  uint32_t br;
//  FIL f;
//  FRESULT fresult = f_open(&f, filename, FA_READ | FA_OPEN_EXISTING);
//  if(fresult != FR_OK)
//    fresult = f_open(&f, "NOIMG.MYF", FA_READ | FA_OPEN_EXISTING);
//  if(fresult == FR_OK)
//  {
//    f_read(&f, buf, bufsize, &br);
//    uint8_t* pos = myf.Draw_MYF_Start(buf, bufsize, x, y);
//    while(pos)
//    {
//      f_read(&f, buf, bufsize, &br);
//      pos = myf.Draw_MYF_Continue(buf, bufsize);
//      n++;
//    }
//    f_close(&f);
//  }  
//}

//void LoadFromFileRAW(char* filename, int x, int y)
//{
//  //int n = 1; 
//  uint32_t br;
//  FIL f;
//  FRESULT fresult = f_open(&f, filename, FA_READ | FA_OPEN_EXISTING);
//  if(fresult != FR_OK)
//    return;
//  
//  f_read(&f, buf, bufsize, &br);
//  CRect r;
//  r.left = 0;
//  r.width = 272;
//  r.top = 0;
//  r.height = 480;
//  lcd.SetDrawRect(&r);
//  lcd.WriteMemoryStart();
//  WritePixelsBitmap2((uint16_t*)buf, (br>>1));
//  while(br)
//  {
//    f_read(&f, buf, bufsize, &br);
////    if(br == 0)
////      r.top = 7;//break;
//    WritePixelsBitmap2((uint16_t*)buf, (br>>1));
//  }
//  f_close(&f);
//}

void zoomIn(uint8_t* zoom)
{
  if(++(*zoom) > MAX_ZOOM)
    *zoom = MAX_ZOOM;
}

void zoomOut(uint8_t* zoom)
{
  if(--(*zoom) < MIN_ZOOM)
    *zoom = MIN_ZOOM;
}

void scrlH(PIXELPOINT_T* pt, signed char dx)
{
  int px = pt->tile_pixl_x + (int)dx;
  pt->pixl_x += (int)dx;
  if(px > 255)
  {
    px-= 256;
    pt->tile_x += 1;
  }
  if(px < 0)
  {
    px+= 256;
    pt->tile_x -= 1;
  }
  pt->tile_pixl_x = px;
}

void scrlV(PIXELPOINT_T* pt, signed char dy)
{
  int t = pt->tile_pixl_y + (int)dy;
  pt->pixl_y += (int)dy;
  if(t > 255)
  {
    t-= 256;
    pt->tile_y += 1;
  }
  if(t < 0)
  {
    t+= 256;
    pt->tile_y -= 1;
  }
  pt->tile_pixl_y = t;
}
