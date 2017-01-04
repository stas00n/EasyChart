/****************************************************************************
* wintypes.h - Shorted variant of                                           *
* windef.h (Basic Windows Type Definitions)  for using windows              *
* types on embedded platforms                                               *
*                                                                           *
* Copyright (c) Microsoft Corporation. All rights reserved.                 *
*                                                                           *
****************************************************************************/


#ifndef _WINTYPES_
#define _WINTYPES_

#ifdef __cplusplus
extern "C" {
#endif


/*
 * BASETYPES is defined in ntdef.h if these types are already defined
 */

#ifndef BASETYPES
#define BASETYPES
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;
#endif  /* !BASETYPES */

#define MAX_PATH          260

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;



typedef int                 INT;
typedef unsigned int        UINT;



#ifdef __cplusplus
}
#endif

#endif /* _WINTYPES_ */

