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
  class AnimEvent;
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
  -----------------------------------------------------
  Base AnimTrack interface. A "track" is a set of keys
  -----------------------------------------------------*/

  class AnimTrack : public Object
  {
    ABSTRACT( AnimTrack, Object,
      491ab1fb,63c6,492c,877d6e1266a8e6bc );

    virtual void serialize( Serializer *s, Uint v )
    {
      Object::serialize( s,v );
      s->data( &firstKey );
    }

    friend class Animation;
    friend class AnimController;

  protected:

    Int firstKey;

  public:

    AnimTrack () : firstKey (0) {}
    virtual Int getNumKeys () = 0;
    virtual void evalAt (Int key1, Int key2, Float keyT) = 0;
    virtual void getValue (void *value) {}
  };

  /*
  ----------------------------------------------------------
  Template AnimTrack interface allows strong-typed storage
  and access to track keys. Traits class defines key type
  and interpolation function.
  ----------------------------------------------------------*/
  
  template <class Traits> class AnimTrackT : public AnimTrack
  {
    CLASS( AnimTrackT, AnimTrack,  0,0,0,0 );

    virtual void serialize( Serializer *s, Uint v )
    {
      AnimTrack::serialize( s,v );
      s->dataArray( &keys );
    };
    
  private:

    typename Traits::Value value;
    ArrayList <typename Traits::Value> keys;

  public:

    void addKey( typename Traits::Value value );
    virtual Int getNumKeys ();

    virtual void evalAt (Int key1, Int key2, Float keyT);
    virtual void getValue (void *value);
    virtual typename Traits::Value getValue ();
  };

  /*
  ----------------------------------------------------------
  Template AnimTrack definitions.
  ----------------------------------------------------------*/

  template <class Traits>
  typename void AnimTrackT<Traits>::evalAt (Int key1, Int key2, Float keyT)
  {
    //Must have at least one key
    if (keys.empty()) return;

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

  class FloatTrackTraits
  { public:
    typedef Float Value;
    inline static Float Interpolate (Float v1, Float v2, Float t);
  };

  class BoolTrackTraits
  { public:
    typedef bool Value;
    inline static bool Interpolate (bool v1, bool v2, Float t);
  };

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

  Float FloatTrackTraits::Interpolate (Float v1, Float v2, Float t)
  {
    //Return linear interpolation
    return (1.0f - t) * v1 + t * v2;
  }

  bool BoolTrackTraits::Interpolate (bool v1, bool v2, Float t)
  {
    //Always return first value
    return v1;
  }

  typedef AnimTrackT< Vec3TrackTraits > Vec3AnimTrack;
  typedef AnimTrackT< QuatTrackTraits > QuatAnimTrack;
  typedef AnimTrackT< FloatTrackTraits > FloatAnimTrack;
  typedef AnimTrackT< BoolTrackTraits > BoolAnimTrack;

  TEMPLATE_CLASS( QuatAnimTrack,  AnimTrack, 8ff2d758,a624,445a,87197d3e14bb22c5 );
  TEMPLATE_CLASS( Vec3AnimTrack,  AnimTrack, d4b943cd,5cce,4b50,8cb7f4f2806c8637 );
  TEMPLATE_CLASS( FloatAnimTrack, AnimTrack, 3412ea4f,2111,481b,bded7f3c19a3b1f1 );
  TEMPLATE_CLASS( BoolAnimTrack,  AnimTrack, 9430611b,8001,47fe,8f4045e94c48b1ad );

  /*
  --------------------------------------------------
  Animation is a collection of tracks.
  --------------------------------------------------*/
  
  class Animation : public Object
  {
    CLASS( Animation, Object,
      94b92ac2,05c3,445a,b70f764c1e76c7ac );

    virtual void serialize( Serializer *s, Uint v )
    {
      Object::serialize( s,v );
      s->data( &kps );
      s->data( &duration );
      s->string( &name );
      s->objectPtrArray( &tracks );
      s->objectPtrArray( &events );
      s->objectPtrArray( &observers );
    }

  private:

    friend class AnimController;

    ArrayList <AnimTrack*> tracks;
    ArrayList <AnimObserver*> observers;
    ArrayList <AnimEvent*> events;

  public:

    Uint kps;
    Float duration;
    CharString name;

    Animation () {}
    virtual ~Animation ();

    void addTrack (AnimTrack *track, Int atKey = 0);
    AnimTrack* getTrack (UintSize t);
    UintSize getNumTracks ();

    void addObserver (AnimObserver *o);
    void addEvent (AnimEvent *e);
  };

  /*
  ------------------------------------------------------------
  Animation event is just a named marker at a specified time
  in animation. Animation observers are notified whenever an
  animation even occurs.
  ------------------------------------------------------------*/
  
  class AnimEvent : public Object
  {
    CLASS( AnimEvent, Object,
      57fbbfa8,750d,4548,9760d20c00d91f58 );

    virtual void serialize( Serializer *s, Uint v )
    {
      Object::serialize( s,v );
      s->string( &name );
      s->data( &time );
    }

  private:
    CharString name;
    Float time;

  public:
    
    AnimEvent ()
      : time (0.0f) {}
    
    AnimEvent (const CharString& newName, Float newTime)
      : name (newName), time (newTime) {}

    const CharString& getName() { return name; }
    Float getTime() { return time; }
  };


  /*
  --------------------------------------------------------
  TrackBinding binds an animation track to an observer
  and specifies the parameter to pass to the observer
  when the animated value changes.
  --------------------------------------------------------*/

  class AnimTrackBinding : public Object
  {
    CLASS( AnimTrackBinding, Object,
      3862134c,e0bc,4283,8eaed509b567cc09 );

    virtual void serialize( Serializer *s, Uint v )
    {
      Object::serialize( s,v );
      s->objectRef( &anim );
      s->data( &track );
      s->data( &param );
    }

  public:

    Animation *anim;
    UintSize track;
    Int param;
    AnimTrackBinding () : anim (NULL), track (0), param (0) {};
  };

  /*
  ---------------------------------------------------------
  ObserverBinding is an inverse of TrackBinding. It binds
  an observer to an animation track. An animation can have
  multiple different observer bounds to its tracks.
  ---------------------------------------------------------*/

  class AnimObserverBinding : public Object
  {
    CLASS( AnimObserverBinding, Object,
      54bbbb8a,1c80,4267,acfd988242b0e707 );

    virtual void serialize( Serializer *s, Uint v )
    {
      Object::serialize( s,v );
      s->objectRef( &observer );
      s->data( &param );
    }

  public:

    AnimObserver *observer;
    Int param;
    AnimObserverBinding () : observer (NULL), param (0) {};
  };

  /*
  ------------------------------------------------------
  Animation observer reacts to changes in the animated
  values of the tracks it observes.
  ------------------------------------------------------*/

  class AnimObserver : public Object
  {
    CLASS( AnimObserver, Object,
      d889cd5e,3c49,48a1,96c371a1ae4a4cfc );

    virtual void serialize( Serializer *s, Uint v )
    {
      Object::serialize( s,v );
      s->objectPtrArray( &bindings );
    }

  private:
    friend class AnimController;

    bool change;
    ArrayList< AnimTrackBinding* > bindings;

  public:
    AnimObserver () : change (false) {};
    virtual ~AnimObserver();

    void bindTrack (Animation *anim, UintSize track, Int param = 0);
    virtual void onValueChanged (AnimTrack *track, Int param) {}
    virtual void onAnyValueChanged () {}
    virtual void onEvent (AnimEvent *e) {}
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
    CLASS( AnimController, Object,
      13849919,ea1d,48e4,b00abe0f05d623d2 );

    virtual void serialize( Serializer *s, Uint v )
    {
      Object::serialize( s,v );
      s->data( &maxTime );
      s->data( &animTime );
      s->data( &playSpeed );
      s->data( &maxLoops );
      s->data( &numLoops );
      s->data( &playing );
      s->data( &paused );
      s->objectRef( &anim );
      s->objectPtrArray( &bindings );
    }

  private:

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
    ArrayList< AnimObserverBinding* > bindings;
    void freeBindings();
    void createBindings();

    //Key interpolation
    UintSize trackIndex;
    UintSize eventIndex;
    typedef LinkedList< UintSize >::Iterator TrackIter;
    LinkedList< UintSize > tracksOnKey;
    ArrayList< AnimObserver* > observersOnKey;
    ArrayList< AnimObserver* > observers;

    void timeToKeys (Animation *a, Float t, Int *k1, Int *k2, Float *kt);
    void localizeKeys (AnimTrack *t, Int k1, Int k2, Int *outK1, Int *outK2);
    void evaluateAnimation ();
    void evaluateTrack (UintSize track, Int key1, Int key2, Float keyT);

    //Callbacks
    AnimCallbackFunc endFunc;
    void *endParam;

  public:

    AnimController ();
    virtual ~AnimController ();

    void bindAnimation (Animation *a);
    void bindObserver (AnimObserver *o);
    void observeAt (Float time);

    void play (Int nloops = 0, Float speed = 1.0f, Float from = 0.0);
    void pause ();
    void unpause ();
    void toggle ();
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
