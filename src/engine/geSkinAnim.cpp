#define GE_API_EXPORT
#include "geEngine.h"

namespace GE
{

  DEFINE_SERIAL_CLASS( SkinAnim,   CLSID_SKINANIM );
  DEFINE_SERIAL_CLASS( SkinTrack,  CLSID_SKINTRACK );


  Quat SkinTrack::evalAt (Float time)
  {
    //Exit soon if just 1 key
    if (keys.size() == 1)
      return keys.first().value;
    
    //Exit soon if time negative
    if (time <= 0.0f)
      return keys.first().value;
    
    //Exit soon if time too large
    if (time >= totalTime)
      return keys.last().value;
    
    //Find 2 keys around the given time
    Float frameCoeff = time / frameTime;
    int k1 = (int) FLOOR( frameCoeff );
    int k2 = (int) CEIL( frameCoeff );
    if (k1 == k2) return keys[ k1 ].value;
    
    //Calculate interpolation coeff
    float t = frameCoeff - (Float)k1;
    
    //Check for the sign flip in the adjacent keys
    Quat q1 = keys[ k1 ].value;
    Quat q2 = keys[ k2 ].value;
    if (Quat::Dot( q1, q2 ) < 0.0f)
    {
      q2.x = -q2.x;
      q2.y = -q2.y;
      q2.z = -q2.z;
      q2.w = -q2.w;
    }

    //Return normalized-linear interpolation
    return Quat::Nlerp( q1, q2, t );
  }

}//namespace GE
