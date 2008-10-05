#define GE_API_EXPORT
#include "geEngine.h"
using namespace OCC;

namespace GE
{

  DEFINE_SERIAL_CLASS (SkinAnim,   CLSID_SKINANIM);
  DEFINE_SERIAL_CLASS (SkinTrack,  CLSID_SKINTRACK);


  Quat SkinTrack::evalAt (Float time)
  {
    //Exit soon if just 1 key
    if (keys->size() == 1)
      return keys->first().value;
    
    //Exit soon if time negative
    if (time <= 0.0f)
      return keys->first().value;
    
    //Exit soon if time too large
    if (time >= totalTime)
      return keys->last().value;
    
    //Find 2 keys around the given time
    Float frameCoeff = time / frameTime;
    int k1 = FLOOR (frameCoeff);
    int k2 = CEIL (frameCoeff);
    if (k1 == k2) return keys->at (k1) .value;
    
    //Calculate interpolation coeff
    float t = frameCoeff - (Float)k1;
    
    //Return spherical-linear interpolation
    return Quat::Slerp (keys->at(k1).value, keys->at(k2).value, t);
  }

}//namespace GE
