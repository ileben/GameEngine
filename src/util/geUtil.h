#ifndef __GEUTIL_H
#define __GEUTIL_H

#include "util/geDefs.h"
#include "util/geClass.h"
#include "util/geSerialize.h"
#include "util/geArraySet.h"
#include "util/geArrayList.h"
#include "util/geHeapArrayList.h"
#include "util/geLinkedList.h"
#include "util/geString.h"
#include "util/geTextParser.h"
#include "util/geTime.h"

namespace GE
{
  /*
  ----------------------------------------------
  Miscelaneous utility functions
  ----------------------------------------------*/
  
  class GE_API_ENTRY Util
  {
  public:
    
    //General math
    template <class T> inline static T Min (const T &a, const T &b);
    template <class T> inline static T Max (const T &a, const T &b);

    //Pointer management
    inline static void PtrSet (void *pptr, UintSize address);
    inline static void PtrAdd (void *pptr, UintSize offset);
    inline static void PtrSub (void *pptr, UintSize offset);
    inline static UintSize PtrDist (void *ptrFrom, void *ptrTo);   
    inline static void* PtrOff (void *ptr, UintSize offset);

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

  UintSize Util::PtrDist (void *ptr1, void *ptr2)
  {
    return ((UintSize)ptr2) - ((UintSize)ptr1);
  }

  void* Util::PtrOff (void *ptr, UintSize offset)
  {
    void *ptrout = ptr;
    PtrAdd (&ptrout, offset);
    return ptrout;
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
#endif//__GEUTIL_H
