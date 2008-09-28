#ifndef __GEDEFS_H
#define __GEDEFS_H

#if defined(WIN32)

//Platform SDK compatibility with VC++ default headers sux
//so we disable "macro already defined" warnings
#  pragma warning (disable : 4005) 
#  include <windows.h>
#  pragma warning (default : 4005)

#endif// defined(WIN32)

//External headers
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <list>
#include <OpenCC/OpenCC.h>


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


namespace GE
{
  typedef char                 Int8;
  typedef unsigned char        Uint8;
  typedef short                Int16;
  typedef unsigned short       Uint16;
  typedef int                  Int32;
  typedef unsigned int         Uint32;
  typedef long long            Int64;
  typedef unsigned long long   Uint64;
  typedef std::size_t          UintP;
  
  typedef float      Float32;
  typedef double     Float64;
  typedef Float32    Float;

  #define PI 3.1415926535f
  #define COS(a)  ((Float)std::cos(a))
  #define SIN(a)  ((Float)std::sin(a))
  #define ACOS(a) ((Float)std::acos(a))
  #define SQRT(a) ((Float)std::sqrt(a))
  #define ASSERT  assert
  #define INLINE  inline
}

#endif// __GEDEFS_H
