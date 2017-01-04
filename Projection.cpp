#include "projection.h"


void LatLon2Pixel(double lat, double lon, BYTE zoom, PIXELPOINT_T* pt)
{
  pt->tiles_total = (1 << (zoom-1));
  pt->pixls_total = (pt->tiles_total) << 8;

  double normX = lon / 360.0 + 0.5;
  double sinLat = sin(lat * 0.01745329252);
  double normY = 0.5 - 0.079577471545948 * log((1 + sinLat) / (1 - sinLat));

  pt->pixl_x = (DWORD)(pt->pixls_total * normX);
  pt->pixl_y = (DWORD)(pt->pixls_total * normY);

  pt->tile_x = pt->pixl_x >> 8;
  pt->tile_y = pt->pixl_y >> 8;

  pt->tile_pixl_x = pt->pixl_x - (pt->tile_x << 8);
  pt->tile_pixl_y = pt->pixl_y - (pt->tile_y << 8);
}

void Pixel2LatLon(PIXELPOINT_T* pt/*DWORD pixlX, DWORD pixlY*/, double* lat, double* lon)
{
  if(pt->pixls_total == 0) return;
  double normX = ((double)pt->pixl_x) / pt->pixls_total;
  double normY = ((double)pt->pixl_y) / pt->pixls_total;

  *lon = 360 * (normX - 0.5);
  double exponent = 12.5663706143592 * (0.5 - normY);// 4 * pi * 0.5 - normY
  double A = exp(exponent); // A = e^(4 * pi * 0.5 - normY)

  *lat = 57.29577951308232 * asin((A - 1) / (A + 1));// lat = asind()
}

bool GenerateTilePath(DWORD x, DWORD y, BYTE zoom, char* rootDir, char* path)
{
  char str[20];
  strcpy(path, rootDir);
  
  strcat(path, "/z");
  _utoa(zoom, str);
  strcat(path, str);
  
  strcat(path, "/");
  _utoa(x >> 10, str);
  strcat(path, str);
  
  strcat(path,"/x");
  _utoa(x, str);
  strcat(path, str);
  
  strcat(path, "/");
  _utoa(y >> 10, str);
  strcat(path, str);
  
  strcat(path,"/y");
  _utoa(y, str);
  strcat(path, str);
  
  strcat(path, ".myf");

  return 1;
}