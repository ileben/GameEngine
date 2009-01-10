#ifndef __GEDEFS_H
#define __GEDEFS_H

#if defined( GE_BUILD_DLL )

  #if defined(WIN32)
  #  define GE_DLL_EXPORT __declspec(dllexport)
  #  define GE_DLL_IMPORT __declspec(dllimport)
  #else
  #  define GE_DLL_EXPORT
  #  define GE_DLL_IMPORT
  #endif

  #if defined(GE_API_EXPORT)
  #  define GE_API_ENTRY GE_DLL_EXPORT
  #else
  #  define GE_API_ENTRY GE_DLL_IMPORT
  #endif

#else

  #define GE_API_ENTRY

#endif


//Platform SDK compatibility with VC++ default headers sux
//so we disable "macro already defined" warnings
#if defined(WIN32)
#  pragma warning (disable : 4005) 
#  include <windows.h>
#  pragma warning (default : 4005)
#endif//defined(WIN32)


//External headers
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cassert>
#include <cmath>
#include <list>
#include <vector>
#include <deque>

#ifndef WIN32
#  include <sys/time.h>
#endif


//General definitions
namespace GE
{
  typedef char                 Int8;
  typedef unsigned char        Uint8;
  typedef unsigned char        Byte;
  typedef short                Int16;
  typedef unsigned short       Uint16;
  typedef int                  Int32;
  typedef unsigned int         Uint32;
  typedef long long            Int64;
  typedef unsigned long long   Uint64;
  typedef std::size_t          UintSize;
  
  typedef float      Float32;
  typedef double     Float64;
  typedef Float32    Float;
  
  #define PI       3.1415926535f
  #define COS(a)   ((Float)std::cos(a))
  #define SIN(a)   ((Float)std::sin(a))
  #define TAN(a)   ((Float)std::tan(a))
  #define ACOS(a)  ((Float)std::acos(a))
  #define SQRT(a)  ((Float)std::sqrt(a))
  #define FLOOR(a) ((Float)std::floor(a))
  #define CEIL(a)  ((Float)std::ceil(a))
  #define ASSERT   assert
  #define INLINE   inline
}

#endif// __GEDEFS_H
