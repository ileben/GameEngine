#ifndef __GEUTIL_H
#define __GEUTIL_H

namespace GE
{
  /*
  ----------------------------------------------
  Miscelaneous utility functions
  ----------------------------------------------*/
  
  class GE_API_ENTRY Util
  {
  public:

    //Pointer management
    inline static void PtrSet (void *pptr, UintP address);
    inline static void PtrAdd (void *pptr, UintP offset);
    inline static void PtrSub (void *pptr, UintP offset);
    inline static UintP PtrDist (void *ptrFrom, void *ptrTo);   
    inline static void* PtrOff (void *ptr, UintP offset);

    //Trigonometry
    inline static Float DegToRad (Float degrees);
    inline static Float RadToDeg (Float radians);
  };
  
  /*
  ---------------------------------------------
  Pointer management
  ---------------------------------------------*/

  void Util::PtrSet (void *pptr, UintP address)
  {
    *((UintP*)pptr) = address;
  }
  
  void Util::PtrAdd (void *pptr, UintP offset)
  {
    *((UintP*)pptr) += offset;
  }
  
  void Util::PtrSub (void *pptr, UintP offset)
  {
    *((UintP*)pptr) -= offset;
  }

  UintP Util::PtrDist (void *ptr1, void *ptr2)
  {
    return ((UintP)ptr2) - ((UintP)ptr1);
  }

  void* Util::PtrOff (void *ptr, UintP offset)
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
