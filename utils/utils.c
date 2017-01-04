#include "utils.h"

// Read unaligned Little Endian 32 bit value
uint32_t Get_LE32(uint8_t* p32)
{
  uint32_t ret = *(p32 + 3); ret <<=8;
  ret |= *(p32 + 2); ret <<=8;
  ret |= *(p32 + 1); ret <<=8;
  return (ret | (*p32));
}

// Read unaligned Big Endian 32 bit value
uint32_t Get_BE32(uint8_t* p32)
{
  uint32_t ret = (*p32++); ret <<= 8;
  ret |= *p32++; ret <<= 8;
  ret |= *p32++; ret <<= 8;
  return (ret | (*p32));
}

