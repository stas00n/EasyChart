#ifndef _PROJECTION_H_
#define _PROJECTION_H_

#include "wintypes.h"
#include <stdint.h>
#include "utils.h"
#include <string.h>
#include <math.h>

typedef struct
{
  DWORD pixls_total;
  DWORD tiles_total;
  DWORD pixl_x;
  DWORD pixl_y;
  DWORD tile_x;
  DWORD tile_y;
  BYTE  tile_pixl_x;
  BYTE  tile_pixl_y;
}PIXELPOINT_T;

void LatLon2Pixel(double lat, double lon, BYTE zoom, PIXELPOINT_T* pt);
void Pixel2LatLon(PIXELPOINT_T* pt/*DWORD pixlX, DWORD pixlY*/, double* lat, double* lon);
bool GenerateTilePath(DWORD x, DWORD y, BYTE zoom, char* rootDir, char* path);





#endif /* _PROJECTION_H_ */