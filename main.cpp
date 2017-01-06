#include "main.h"
#include "ff.h"

extern const unsigned short up[];
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

//  int dx = 0;
//  CRect rect;
//  rect.left = dx+14;
//  rect.width = 2;
//  rect.top = 0;
//  rect.height = 480;
  
  lcd.Clear();
  
  lcd._bkCol = 0xF800;
  lcd._penCol = 0xFFFF;
  // Print() test
  while(1)
  {
    lcd._font = (font_t*)BigFont;
    lcd.Print("Hello, World!", 50, 200);
    lcd._font = (font_t*)SevenSegNumFont;
    lcd.Print("1234", 50, 240);
    lcd.Clear(0x1f);
  }
  // Center tile origin
  int centerTileX,  centerTileY;
  
  // Find center tile
  LatLon2Pixel(lat, lon, zoom, &pt);
  
  // Create sprite
  CSprite sp;
  sp.Create(20, 20, (uint16_t*)up);
  // Set sprite transparent color
  sp._trColor = 0;
  
  while(1)
  {

    
    uint32_t b = GetButton();
    switch(b)
    {
    case 1:
      scrlV(&pt, (signed char)15);
      break;
      
    case 2:
      scrlV(&pt, (signed char)-15);
      break;
      
    case 3:
      Pixel2LatLon(&pt, &lat, &lon);
      zoomIn(&zoom);
      LatLon2Pixel(lat, lon, zoom, &pt);
      break;
      
    case 4:
      Pixel2LatLon(&pt, &lat, &lon);
      zoomOut(&zoom);
      LatLon2Pixel(lat, lon, zoom, &pt);
      break;
      
    default:
      break;
    }
    Clear_Buttons();
    
    
    GenerateTilePath(pt.tile_x, pt.tile_y,zoom, "SONAR", filepath);
    
    centerTileX = 136 - pt.tile_pixl_x;
    centerTileY = 240 - pt.tile_pixl_y;
    
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
        LoadFromFile(filepath, centerTileX + 256 * tx, centerTileY + 256 * ty);
      }
    }
    
    // Sprite drawing test:
    for(int i = 0; i < 100; i++)
    {
      lcd.DrawSprite(&sp, 100 + i, 210);
      __delay(100000);
    }
    lcd.ClearSprite(&sp);
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
