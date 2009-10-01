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

  class Animation;
  class AnimTrack;
  class AnimObserver;
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

  class AnimTrack : public Object
  {
    DECLARE_SUBABSTRACT( AnimTrack, Object );
    DECLARE_DATAVAR( firstKey );
    DECLARE_END;

    friend class Animation;
    friend class AnimController;

  protected:

    Int firstKey;

  public:

    AnimTrack (SM *sm) : Object (sm) {}
    AnimTrack () : firstKey (0) {}

    virtual Int getNumKeys () = 0;
    virtual void evalAt (Int key1, Int key2, Float keyT) = 0;
    virtual void getValue (void *value) {}
  };
  
  template <class Traits> class AnimTrackT : public AnimTrack
  {
    DECLARE_SERIAL_SUBCLASS( AnimTrackT, AnimTrack );
    DECLARE_OBJVAR( keys );
    DECLARE_END;
    
  private:
    typename Traits::Value value;
    ArrayList <typename Traits::Value> keys;

  public:
    AnimTrackT (SM *sm) : AnimTrack(sm), keys(sm) {}
    AnimTrackT () {}

    void addKey( typename Traits::Value value );
    virtual Int getNumKeys ();

    virtual void evalAt (Int key1, Int key2, Float keyT);
    virtual void getValue (void *value);
    virtual typename Traits::Value getValue ();
  };

  /*
  -------------------------------------------
  Template implementations
  -------------------------------------------*/

  template <class Traits>
  typename void AnimTrackT<Traits>::evalAt (Int key1, Int key2, Float keyT)
  {
    //Must have at least one key
    if (keys.empty()) return;

    //Localize keys and clamp to range
    key1 = Util::Clamp( key1 - firstKey, 0, (Int)keys.size()-1 );
    key2 = Util::Clamp( key2 - firstKey, 0, (Int)keys.size()-1 );

    //Check if keys the same
    if (key1 == key2)
    {
      //Return exact key value
      value = keys[ key1 ];
    }
    else
    {
      //Interpolate between two key values
      value = Traits::Interpolate(
        keys[ key1 ],
        keys[ key2 ],
        keyT );
    }
  }

  template <class Traits>
  void AnimTrackT< Traits >::addKey (typename Traits::Value value)
  {
    keys.pushBack( value );
  }

  template <class Traits>
  void AnimTrackT< Traits >::getValue (void *outValue)
  {
    *((typename Traits::Value*) outValue) = value;
  }

  template <class Traits>
  typename Traits::Value AnimTrackT< Traits >::getValue ()
  {
    return value;
  }

  template <class Traits>
  Int AnimTrackT< Traits >::getNumKeys ()
  {
    return (Int) keys.size();
  }


  /*
  -------------------------------------------
  Traits for quaternion and vector tracks
  -------------------------------------------*/

  class QuatTrackTraits
  { public:
    typedef Quat Value;
    inline static Quat Interpolate (Quat q1, Quat q2, Float t);
  };

  class Vec3TrackTraits
  { public:
    typedef Vector3 Value;
    inline static Vector3 Interpolate (const Vector3 &v1, const Vector3 &v2, Float t);
  };

  typedef AnimTrackT< Vec3TrackTraits > Vec3AnimTrack;
  typedef AnimTrackT< QuatTrackTraits > QuatAnimTrack;


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
  Anim event
  --------------------------------------------------*/
  
  class AnimEvent : public Object
  {
    DECLARE_SERIAL_SUBCLASS( AnimEvent, Object );
    DECLARE_OBJVAR( name );
    DECLARE_DATAVAR( time );
    DECLARE_END;

  private:
    CharString name;
    Float time;

  public:
    AnimEvent (SM *sm) : Object (sm), name (sm) {}
    AnimEvent () : time (0.0f) {}
    AnimEvent (const CharString& newName, Float newTime)
      : name (newName), time (newTime) {}
    const CharString& getName() { return name; }
    Float getTime() { return time; }
  };

  /*
  --------------------------------------------------
  Animation is a group of tracks
  --------------------------------------------------*/
  
  class Animation : public Object
  {
    DECLARE_SERIAL_SUBCLASS( Animation, Object );
    DECLARE_DATAVAR( kps );
    DECLARE_DATAVAR( duration );
    DECLARE_OBJVAR( name );
    DECLARE_OBJVAR( tracks );
    DECLARE_OBJVAR( observers );
    DECLARE_END;

    friend class AnimController;

  private:

    ObjPtrArrayList <AnimTrack> tracks;
    ObjPtrArrayList <AnimObserver> observers;
    ObjPtrArrayList <AnimEvent> events;

  public:

    Uint kps;
    Float duration;
    CharString name;

    Animation (SM *sm) : Object(sm), name(sm), tracks(sm), observers(sm), events(sm) {}
    Animation () {}
    virtual ~Animation ();

    void addTrack (AnimTrack *track, Int atKey = 0);
    AnimTrack* getTrack (UintSize t);
    UintSize getNumTracks ();

    void addObserver (AnimObserver *o);
    void addEvent (AnimEvent *e);
  };


  /*
  --------------------------------------------------------
  TrackBinding binds an animation track to an observer
  and specifies the parameter to pass to the observer
  when the animated value changes.
  --------------------------------------------------------*/

  class AnimTrackBinding : public Object
  {
    DECLARE_SERIAL_SUBCLASS( AnimTrackBinding, Object );
    DECLARE_OBJREF( anim );
    DECLARE_DATAVAR( track );
    DECLARE_DATAVAR( param );
    DECLARE_END;

  public:

    Animation *anim;
    UintSize track;
    Int param;

    AnimTrackBinding () : anim (NULL), track (0), param (0) {};
    AnimTrackBinding (SM *sm) : Object (sm) {};
  };

  /*
  ------------------------------------------------------
  Animation observer reacts to changes in the animated
  values of the tracks it observes.
  ------------------------------------------------------*/

  class AnimObserver : public Object
  {
    DECLARE_SERIAL_SUBCLASS( AnimObserver, Object );
    DECLARE_OBJVAR( bindings );
    DECLARE_END

    friend class AnimController;

  private:
    bool change;
    ObjPtrArrayList< AnimTrackBinding > bindings;

  public:
    AnimObserver () : change (false) {};
    AnimObserver (SM *sm) : Object (sm), change (false) {}
    virtual ~AnimObserver();

    void bindTrack (Animation *anim, UintSize track, Int param = 0);
    virtual void onValueChanged (AnimTrack *track, Int param) {}
    virtual void onAnyValueChanged () {}
    virtual void onEvent (AnimEvent *e) {}
  };

  /*
  ---------------------------------------------------------
  ObserverBinding is an inverse of a TrackBinding.
  ---------------------------------------------------------*/

  class AnimObserverBinding : public Object
  {
    DECLARE_SERIAL_SUBCLASS( AnimObserverBinding, Object );
    DECLARE_OBJREF( observer );
    DECLARE_DATAVAR( param );
    DECLARE_END;

  public:

    AnimObserver *observer;
    Int param;

    AnimObserverBinding () : observer (NULL), param (0) {};
    AnimObserverBinding (SM *sm) : Object (sm) {}
  };


  /*
  ----------------------------------------------------
  Controller provides control over animation playback
  and notifies the animation observers when values on
  the animation tracks change.
  ----------------------------------------------------*/

  typedef void (*AnimCallbackFunc) (AnimController *ctrl, void *param);

  class AnimController : public Object
  {
    DECLARE_SERIAL_SUBCLASS( AnimController, Object );
    DECLARE_DATAVAR( maxTime );
    DECLARE_DATAVAR( animTime );
    DECLARE_DATAVAR( playSpeed );
    DECLARE_DATAVAR( maxLoops );
    DECLARE_DATAVAR( numLoops );
    DECLARE_DATAVAR( playing );
    DECLARE_DATAVAR( paused );
    DECLARE_OBJVAR( bindings );
    DECLARE_OBJREF( anim );
    DECLARE_END;

    //Time control
    Float maxTime;
    Float animTime;
    Float playSpeed;
    Int maxLoops;
    Int numLoops;
    bool playing;
    bool paused;

    //Animation bindings
    Animation *anim;
    ObjPtrArrayList< AnimObserverBinding > bindings;
    void freeBindings();
    void createBindings();

    //Key interpolation
    Int key1;
    Int key2;
    Float keyT;
    UintSize trackIndex;
    typedef LinkedList< UintSize >::Iterator TrackIter;
    LinkedList< UintSize > tracksOnKey;
    ArrayList< AnimObserver* > observersOnKey;
    
    void evaluateAnimation ();
    void evaluateTrack (UintSize track);

    //Callbacks
    AnimCallbackFunc endFunc;
    void *endParam;

  public:

    AnimController ();
    AnimController (SM *sm);
    virtual ~AnimController ();

    void bindAnimation (Animation *a);
    void bindObserver (AnimObserver *o);

    void play (Int nloops = 0, Float speed = 1.0f, Float from = 0.0);
    void pause ();
    void unpause ();
    void stop ();
    void tick ();

    void setTime (Float time);
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
