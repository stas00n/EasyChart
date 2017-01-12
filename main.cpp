#include "main.h"
#include "ff.h"
#include "ffasync.h"

extern const unsigned short up[];
const int bufsize = 2048;

CLCD lcd;
CMYF myf;
uint8_t* buf0;
uint8_t* buf1;
  void main()
{
  Clock_Config();
  GPIO_Config();
  
  
  Buttons_Init();
  lcd.Init();
  lcd.Clear();
  lcd._bkCol = 0;
  lcd._font = (FONT_INFO*)&arialNarrow_10ptFontInfo;
  lcd._penCol = 0xffff;
  struct{
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
    lcd.Print("Error: Can't initialize SD Card!", con.x, con.y);
  else
    lcd.Print("SD card init OK.", con.x, con.y);
  
  con.y += lcd._font->height;
  buf0 = (uint8_t*)malloc(bufsize);
  buf1 = (uint8_t*)malloc(bufsize);
memset(buf0, 0, 2048);  
memset(buf1, 0, 2048); 
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
    UINT br;
    uint8_t* rbuf = buf1;
    uint8_t* dbuf = buf0;
    f_read(&f, buf0, bufsize, &br);
    //f_aread_start(&f, buf0, bufsize, &br);
    while((fresult = f_aread_getrdy()) != FR_OK);
    //f_aread_start(&f, buf1, bufsize, &br);
    
    
    uint8_t* pos = myf.Draw_MYF_Start(buf0, bufsize, 0, 0);

    while(br)
    {
//      if(rbuf == buf0)
//        rbuf = buf1;
//      else rbuf = buf0;
//      
//      if(dbuf == buf0)
//        dbuf = buf1;
//      else dbuf = buf0;
      f_read(&f, buf0, bufsize, &br);
      //f_aread_start(&f, rbuf, bufsize, &br);
      //while((fresult = f_aread_getrdy()) != FR_OK);
      pos = myf.Draw_MYF_Continue(buf0, bufsize);
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
