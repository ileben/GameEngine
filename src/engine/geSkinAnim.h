#ifndef __GESKELANIM_H
#define __GESKELANIM_H

#include "util/geUtil.h"
#include "geVectors.h"

namespace GE
{
  /*
  -----------------------------------
  Track is a set of keys
  -----------------------------------*/
  
  template <class Traits> class AnimTrack
  {
    DECLARE_SERIAL_CLASS( AnimTrack );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;
    
  public:
    Float32 totalTime;
    Float32 frameTime;
    ArrayList <typename Traits::Key> keys;
    
    void serialize (void *sm)
    {
      ((SM*)sm)->dataVar( &totalTime );
      ((SM*)sm)->dataVar( &frameTime );
      ((SM*)sm)->objectVar( &keys );
    }

    AnimTrack (SM *sm) : keys (sm) {}
    AnimTrack () : totalTime(0.0f), frameTime(0.0f) {}

    typename Traits::Value evalAt (Float time);
  };

  /*
  -------------------------------------------
  Traits for Quaternion and Vector3 tracks
  -------------------------------------------*/

  class QuatTrackTraits
  { public:
    typedef Quat Value;
    struct Key { Quat value; };
    inline static Quat Interpolate (Quat q1, Quat q2, Float t);
  };

  class Vec3TrackTraits
  { public:
    typedef Vector3 Value;
    struct Key { Vector3 value; };
    inline static Vector3 Interpolate (const Vector3 &v1, const Vector3 &v2, Float t);
  };

  typedef AnimTrack< Vec3TrackTraits > Vec3AnimTrack;
  typedef AnimTrack< QuatTrackTraits > QuatAnimTrack;


  /*
  -------------------------------------------
  Template implementations
  -------------------------------------------*/

  template <class Traits>
  typename Traits::Value AnimTrack<Traits>::evalAt (Float time)
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
    return Traits::Interpolate(
      keys[ k1 ].value,
      keys[ k2 ].value,
      t );
  }

  Quat QuatTrackTraits::Interpolate (Quat q1, Quat q2, Float t)
  {
    //Check for sign flip in the adjacent keys
    if (Quat::Dot( q1, q2 ) < 0.0f) {
      q2.x = -q2.x;
      q2.y = -q2.y;
      q2.z = -q2.z;
      q2.w = -q2.w;
    }
    
    //Return normalized-linear interpolation
    return Quat::Nlerp( q1, q2, t );
  }

  Vector3 Vec3TrackTraits::Interpolate (const Vector3 &v1, const Vector3 &v2, Float t)
  {
    //Return linear interpolation
    return Vector::Lerp( v1, v2, t );
  }


  /*
  -----------------------------------
  Animation is a set of tracks
  -----------------------------------*/
  
  class SkinAnim
  {
    DECLARE_SERIAL_CLASS( SkinAnim );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;
    
  public:
    CharString name;
    Float duration;
    ClassArrayList <Vec3AnimTrack> tracksT;
    ClassArrayList <QuatAnimTrack> tracksR;
    
    void serialize (void *sm)
    {
      ((SM*)sm)->objectVar( &name );
      ((SM*)sm)->dataVar( &duration );
      ((SM*)sm)->objectVar( &tracksT );
      ((SM*)sm)->objectVar( &tracksR );
    }
    
    SkinAnim (SM *sm) : name (sm), tracksT (sm), tracksR(sm) {}
    SkinAnim () {}
    ~SkinAnim ()
    {
      for (UintSize t=0; t<tracksT.size(); ++t)
        delete tracksT[t];
      for (UintSize t=0; t<tracksR.size(); ++t)
        delete tracksR[t];
    }
    
    void evalFrame (Float time);
  };
};

#endif//__GESKELANIM_H
