#ifndef _FONTS_H_
#define _FONTS_H_

#include <stdint.h>

typedef const struct 
{
  uint16_t              chWidth;
  uint16_t              chOffset;
}FONT_CHAR_INFO;

typedef struct
{
  uint8_t               height ;        //  Character height
  char                  startCh;        //  Start character
  char                  endCh;          //  End character
  uint8_t               spaceW;         //  Width, in pixels, of space character
  FONT_CHAR_INFO*       desc;           //  Character descriptor array
  const uint8_t*        bitmaps;        //  Character bitmap array 
  uint8_t               interval;
}FONT_INFO;



#include "Arial_Narrow_10_b.h"

#endif /* _FONTS_H_ */