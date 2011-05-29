#ifndef __GEMISC_H
#define __GEMISC_H

namespace GE
{
  /*
  ----------------------------------------------
  Miscelaneous utility functions
  ----------------------------------------------*/
  
  class Util
  {
  public:
    
    //General math
    template <class T> inline static T Min (const T &a, const T &b);
    template <class T> inline static T Max (const T &a, const T &b);
    template <class T> inline static T Clamp (const T &a, const T &min, const T &max);

    //Pointer management
    inline static void PtrSet (void *pptr, UintSize address);
    inline static void PtrAdd (void *pptr, UintSize offset);
    inline static void PtrSub (void *pptr, UintSize offset);
    inline static UintSize PtrDist (const void *ptrFrom, const void *ptrTo);
    inline static void* PtrOff (const void *ptr, UintSize offset);

    //Trigonometry
    inline static Float DegToRad (Float degrees);
    inline static Float RadToDeg (Float radians);
  };

  /*
  ---------------------------------------------
  General math
  ---------------------------------------------*/

  template <class T> T Util::Min (const T &a, const T &b)
  {
    return (a <= b) ? a : b;
  }
  
  template <class T> T Util::Max (const T &a, const T &b)
  {
    return (a >= b) ? a : b;
  }

  template <class T> T Util::Clamp (const T &a, const T &min, const T &max)
  {
    return Util::Min( Util::Max( a, min ), max );
  }
  
  /*
  ---------------------------------------------
  Pointer management
  ---------------------------------------------*/

  void Util::PtrSet (void *pptr, UintSize address)
  {
    *((UintSize*)pptr) = address;
  }
  
  void Util::PtrAdd (void *pptr, UintSize offset)
  {
    *((UintSize*)pptr) += offset;
  }
  
  void Util::PtrSub (void *pptr, UintSize offset)
  {
    *((UintSize*)pptr) -= offset;
  }

  UintSize Util::PtrDist (const void *ptr1, const void *ptr2)
  {
    return ((UintSize)ptr2) - ((UintSize)ptr1);
  }

  void* Util::PtrOff (const void *ptr, UintSize offset)
  {
    const void *ptrout = ptr;
    PtrAdd (&ptrout, offset);
    return (void*) ptrout;
  }

  /*
  ---------------------------------------------
  Trigonometry
  ---------------------------------------------*/

  Float Util::DegToRad (Float degrees)
  {
    return PI * degrees / 180;
  }

  Float Util::RadToDeg (Float radians)
  {
    return 180 * radians / PI;
  }

}//namespace GE
#endif//__GEMISC_H
