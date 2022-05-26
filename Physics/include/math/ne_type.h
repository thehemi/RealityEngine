#ifndef NE_TYPE_H
#define NE_TYPE_H

#include <stdarg.h>
#include <tchar.h>

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#ifdef NULL
#undef NULL
#endif 

#ifdef TRUE
#undef TRUE
#endif 

#ifdef FALSE
#undef FALSE
#endif

///////////////////////////////////////////////////////////////////////////

#define FALSE       0                   // make sure that we know what false is
#define TRUE        1                   // Make sure that we know what true is
#define NULL        0                   // Make sure that null does have a type
#define inline      __forceinline       // Make sure that the compiler inlines when we tell him

///////////////////////////////////////////////////////////////////////////
// BASIC TYPES
///////////////////////////////////////////////////////////////////////////

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned __int64    u64;
typedef signed   char       s8;
typedef signed   short      s16;
typedef signed   int        s32;
typedef signed   __int64    s64;
typedef float               f32;
typedef double              f64;
typedef u8                  neByte;
typedef s32                 neErr;
typedef s32                 neBool;

const char PATH_SEP = '\\';

#endif //NE_TYPE_H
