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
  
  double lat = 54.679;
  double lon = 20.349;
  uint8_t zoom = 13;  
  PIXELPOINT_T pt;
  
  char filepath[MAX_PATH];
 // GenerateTilePath(pt.tile_x, pt.tile_y,zoom, "SONAR", filepath);
//  int dx = 0;
//  CRect rect;
//  rect.left = dx+14;
//  rect.width = 2;
//  rect.top = 0;
//  rect.height = 480;
  
  lcd.Clear();
  int dx = 0,dy = 0;
  // position to draw center tile
  int centerTileX,  centerTileY;
    
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
      zoomIn(&zoom);
      //dy += 2;
      break;
    case 4:
      zoomOut(&zoom);
      //dy -= 2;
      break;
    default:
      break;
    }
    Clear_Buttons();
    
    // Find center tile
    LatLon2Pixel(lat, lon, zoom, &pt);
    GenerateTilePath(pt.tile_x, pt.tile_y,zoom, "SONAR", filepath);
    
    centerTileX = 136 - pt.tile_pixl_x;
    centerTileY = 240 - pt.tile_pixl_y;
   // LoadFromFile(filepath, dx + centerTileX, dy + centerTileY);
    
    int toLeft;
    if(centerTileX > 0) toLeft = -1;
    else toLeft = 0;
    
    int toRight;
    if(centerTileX + 256 < 272) toRight = 1;
    else toRight = 0;
    
    int upper;
    if(centerTileY > 0) upper = -1;
    else upper = 0;

    int lower;
    if(centerTileY < 480) lower = 1;
    else lower = 0;
    
    for(int tx = toLeft; tx <= toRight; tx++)
    {
      for (int ty = upper; ty <= lower; ty++)
      {
        GenerateTilePath(pt.tile_x + tx, pt.tile_y + ty,zoom, "SONAR", filepath);
        LoadFromFile(filepath, dx + centerTileX + 256 * tx, dy + centerTileY + 256 * ty);
      }
    }
  }
}

void LoadFromFile(char* filename, int x, int y)
{
  int n = 1; 
  uint32_t br;
  FIL f;
  FRESULT fresult = f_open(&f, filename, FA_READ | FA_OPEN_EXISTING);
  if(fresult != FR_OK)
    fresult = f_open(&f, "NOIMG.MYF", FA_READ | FA_OPEN_EXISTING);
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

