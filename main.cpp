#include "main.h"
#include "ff.h"


const int bufsize = 2048;

CLCD lcd;
CMYF myf;
uint8_t* buf;

void main()
{
  Clock_Config();
  GPIO_Config();
  
  Buttons_Init();
  lcd.Init();
  
  FRESULT fresult;
  FATFS fs;
  fresult = f_mount(&fs, "0", 1);
  

  buf = (uint8_t*)malloc(bufsize);
  

  
//  int dx = 0;
//  CRect rect;
//  rect.left = dx+14;
//  rect.width = 2;
//  rect.top = 0;
//  rect.height = 480;
  
  lcd.Clear();
  int dx = 0,dy = 0;
  while(1)
  {

    
    uint32_t b = GetButton();
    switch(b)
    {
    case 1:
      dx += 2;
      break;
    case 2:
      dx -= 2;
      break;
    case 3:
      dy += 2;
      break;
    case 4:
      dy -= 2;
      break;
    default:
      break;
    }
    Clear_Buttons();

      LoadFromFile("x4548/y2605.myf", dx-100, dy-10);
      LoadFromFile("x4548/y2606.myf", dx-100, dy+246);
      LoadFromFile("x4549/y2605.myf", dx+156, dy-10);
      LoadFromFile("x4549/y2606.myf", dx+156, dy+246);

    
  }
}

void LoadFromFile(char* filename, int x, int y)
{
  int n = 1; 
  uint32_t br;
  FIL f;
  FRESULT fresult = f_open(&f, filename, FA_READ | FA_OPEN_EXISTING);
  if(fresult == FR_OK)
  {
    f_read(&f, buf, bufsize, &br);
    uint8_t* pos = myf.Draw_MYF_Start(buf, bufsize, x, y);
    while(pos)
    {
      f_read(&f, buf, bufsize, &br);
      pos = myf.Draw_MYF_Continue(buf, bufsize);
      n++;
    }
    f_close(&f);
  }  
}

