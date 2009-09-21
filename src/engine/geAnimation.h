#ifndef __GEANIMATION_H
#define __GEANIMATION_H

#include "util/geUtil.h"
#include "math/geVectors.h"

namespace GE
{

  /*
  -----------------------------------
  Forward declarations
  -----------------------------------*/

  class AnimController;


  /*
  -----------------------------------
  Timed Keys (currently not used)
  -----------------------------------*/
  /*
  template <class T> class TimedKey
  {
    Float32 time;
    T value;
  };
  
  template <class T> class TimedTrack
  {
    ArrayListT <TimedKey<T> > *keys;
    T evalAt (Float time, int keyHint = 0, int *outKeyHint = NULL);
  };

  template <class T> class
  T SkinTrack<T>::evalAt (Float time,
                          int keyHint,
                          int *outKeyHint)
  {
    //Exit soon if just 1 key
    if (keys->size() == 1)
      return keys->first().value;
    
    //Exit soon if time negative
    if (time <= 0.0f)
      return keys->first().value;
    
    //Exit soon if time too large
    if (time >= keys->last().time)
      return keys->last().value;
    
    //Start searching at the hinted key
    int key = keyHint >= keys->size() ? 0 : keyHint;
    
    //Make sure we start to search before the time sought
    if (keys->at (key) .time > time)
      key = 0;
    
    //Find first key with greater or equal time
    while (keys->at (key+1) .time < time)
      key++;
    
    //Calculate interpolation coeff
    SkinKey &key1 = keys->at (key);
    SkinKey &key2 = keys->at (key+1);
    Float alpha = (time - key1.time) / (key2.time - key1.time);
    
    //Output the key hint
    if (outKeyHint != NULL)
      *outKeyHint = key;
    
    //Here's where you return the interpolated value
  }
  */

  /*
  -----------------------------------
  Track is a set of keys
  -----------------------------------*/

  class IAnimTrack : public Object
  {
    DECLARE_SUBABSTRACT( IAnimTrack, Object );
    DECLARE_DATAVAR( totalTime );
    DECLARE_DATAVAR( frameTime );
    DECLARE_END;

  public:
    Float32 totalTime;
    Float32 frameTime;
    IAnimTrack (SM *sm) : Object (sm) {}
    IAnimTrack () : totalTime (0.0f), frameTime (0.0f) {}
    virtual void evalAt (Float time, void *value) = 0;
  };
  
  template <class Traits> class AnimTrack : public IAnimTrack
  {
    DECLARE_SERIAL_SUBCLASS( AnimTrack, IAnimTrack );
    DECLARE_OBJVAR( keys );
    DECLARE_END;
    
  public:
    ArrayList <typename Traits::Key> keys;

    AnimTrack (SM *sm) : IAnimTrack(sm), keys(sm) {}
    AnimTrack () {}

    virtual void evalAt (Float time, void *value);
    typename Traits::Value evalAt (Float time);
  };


  /*
  -------------------------------------------
  Traits for quaternion and vector tracks
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
  void AnimTrack<Traits>::evalAt (Float time, void *value)
  {
    //Return type-specific evaluation
    *((typename Traits::Value*) value) = evalAt( time );
  }

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

    //Robustness check (totalTime might be too long)
    if ((UintSize) k1 >= keys.size() ||
        (UintSize) k2 >= keys.size())
      return keys.last().value;

    //Exit soon if keys the same
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
  --------------------------------------------------
  Controller provides control over animation playback
  --------------------------------------------------*/

  typedef void (*AnimCallbackFunc) (AnimController *ctrl, void *param);

  class AnimController
  {
    Float maxTime;
    Float startTime;
    Float animTime;
    Float playSpeed;
    Int maxLoops;
    Int numLoops;
    bool playing;
    bool paused;

    AnimCallbackFunc endFunc;
    void *endParam;

  public:

    AnimController();
    void play (Float length, Float from = 0.0, Int nloops = 0, Float speed = 1.0f);
    void play ();
    void pause ();
    void unpause ();
    void stop ();
    void tick ();

    void setTime (Float time);
    void setLength (Float length);
    void setSpeed (Float speed);
    void setNumLoops (Int nloops);
    void setEndCallback (AnimCallbackFunc func, void *param);

    Float getTime ();
    Float getDuration ();
    Float getSpeed ();
    Int getNumLoops ();
    Int getNumLoopsLeft ();
    Int getNumLoopsDone ();
    bool isPlaying();
    bool isPaused();
  };


}//namespace GE
#endif//__GEANIMATION_H
