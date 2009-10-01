#include "engine/geAnimation.h"
#include "engine/geKernel.h"

namespace GE
{

  DEFINE_CLASS( AnimTrack );
  DEFINE_SERIAL_TEMPL_CLASS( QuatAnimTrack,  ClassID (0x8ff2d758u, 0xa624, 0x445a, 0x87197d3e14bb22c5ull) );
  DEFINE_SERIAL_TEMPL_CLASS( Vec3AnimTrack,  ClassID (0xd4b943cdu, 0x5cce, 0x4b50, 0x8cb7f4f2806c8637ull) );
  
  DEFINE_SERIAL_CLASS( Animation,           ClassID( 0x974f85e5u, 0x72dd, 0x475c, 0x97e5e2d6e7711a61ull ));
  DEFINE_SERIAL_CLASS( AnimEvent,           ClassID( 0xd744289fu, 0xd525, 0x40de, 0x9b4e90de7fa22742ull ));
  DEFINE_SERIAL_CLASS( AnimObserver,        ClassID( 0x396b2925u, 0x502d, 0x45d6, 0xaf50f072f0eef03dull ));
  DEFINE_SERIAL_CLASS( AnimTrackBinding,    ClassID( 0x6ed45538u, 0x0d1d, 0x46d0, 0xa9515a77ffd53b37ull ));
  DEFINE_SERIAL_CLASS( AnimObserverBinding, ClassID( 0xe12e3c68u, 0x289d, 0x45ff, 0x98f3607fe1c29b1dull ));
  DEFINE_SERIAL_CLASS( AnimController,      ClassID( 0x9cfdb22eu, 0xe70f, 0x4211, 0x90003ba27feade9eull ));


  Animation::~Animation ()
  {
    for (UintSize t=0; t<tracks.size(); ++t)
      delete tracks[ t ];

    for (UintSize o=0; o<observers.size(); ++o)
      delete observers[ o ];

    for (UintSize e=0; e<events.size(); ++e)
      delete events[ e ];
  }

  void Animation::addTrack (AnimTrack *track, Int atKey)
  {
    track->firstKey = atKey;
    tracks.pushBack( track );
  }

  UintSize Animation::getNumTracks ()
  {
    return tracks.size();
  }

  AnimTrack* Animation::getTrack (UintSize t)
  {
    if (t >= tracks.size()) return NULL;
    return tracks[ t ];
  }

  void Animation::addObserver (AnimObserver *o)
  {
    observers.pushBack( o );
  }


  void AnimObserver::bindTrack (Animation *anim, UintSize track, Int param)
  {
    AnimTrackBinding *bind = new AnimTrackBinding;
    bind->anim = anim;
    bind->track = track;
    bind->param = param;
    bindings.pushBack( bind );
  }

  AnimObserver::~AnimObserver()
  {
    for (UintSize b=0; b<bindings.size(); ++b)
      delete bindings[ b ];
  }


  AnimController::AnimController()
  {
    maxTime = 0.0f;
    animTime = 0.0f;
    playSpeed = 1.0f;
    maxLoops = 0;
    numLoops = 0;
    endFunc = NULL;

    anim = NULL;
    trackIndex = 0;
  }

  AnimController::AnimController (SM *sm)
    : Object (sm), bindings (sm)
  {
    endFunc = NULL;
    trackIndex = 0;
  }

  AnimController::~AnimController ()
  {
    freeBindings();
  }

  void AnimController::freeBindings()
  {
    //Delete old bindings
    for (UintSize b=0; b<bindings.size(); ++b)
      delete bindings[ b ];

    bindings.clear();
  }

  void AnimController::createBindings()
  {
    //Create new bindings
    for (UintSize t=0; t<anim->tracks.size(); ++t)
      bindings.pushBack( new AnimObserverBinding() );

    //Bind default animation observers
    for (UintSize o=0; o<anim->observers.size(); ++o)
      bindObserver( anim->observers[ o ] );
  }

  void AnimController::bindObserver (AnimObserver *obsrv)
  {
    //Walk the list of track bindings on the observer
    for (UintSize b=0; b < obsrv->bindings.size(); ++b)
    {
      //Find bindings that target the same animation as the controller
      AnimTrackBinding *trackBind = obsrv->bindings[ b ];
      if (trackBind->anim != anim) continue;

      //Check that the track is within range
      if (trackBind->track > bindings.size()) continue;

      //Bind backwards (track -> observer)
      bindings[ trackBind->track ]->observer = obsrv;
      bindings[ trackBind->track ]->param = trackBind->param;
    }
  }

  void AnimController::bindAnimation (Animation *a)
  {
    stop();

    anim = a;
    maxTime = anim->duration;

    freeBindings();
    createBindings();
  }

  void AnimController::play (Int nloops, Float speed, Float from)
  {
    animTime = Util::Clamp( from, 0.0f, maxTime );
    maxLoops = nloops;
    playSpeed = speed;

    playing = true;
    paused = false;

    trackIndex = 0;
  }

  void AnimController::stop ()
  {
    playing = false;
    paused = false;
    animTime = 0.0;
    numLoops = 0;

    trackIndex = 0;
  }

  void AnimController::pause ()
  {
    if (playing)
      paused = true;
  }

  void AnimController::unpause ()
  {
    if (playing)
      paused = false;
  }

  void AnimController::setTime (Float time)
  {
    animTime = Util::Clamp( time, 0.0f, maxTime );
  }

  void AnimController::setSpeed (Float speed)
  {
    playSpeed = speed;
  }

  void AnimController::setNumLoops (Int nloops)
  {
    maxLoops = nloops;
  }

  void AnimController::setEndCallback (AnimCallbackFunc func, void *param)
  {
    endFunc = func;
    endParam = param;
  }

  Float AnimController::getTime () {
    return animTime;
  }

  Float AnimController::getDuration () {
    return maxTime;
  }

  Float AnimController::getSpeed () {
    return playSpeed;
  }

  Int AnimController::getNumLoops () {
    return maxLoops;
  }

  Int AnimController::getNumLoopsLeft () {
    return maxLoops - numLoops;
  }

  Int AnimController::getNumLoopsDone () {
    return numLoops;
  }

  bool AnimController::isPlaying() {
    return playing;
  }

  bool AnimController::isPaused() {
    return paused;
  }

  void AnimController::tick ()
  {
    if (!playing || paused)
      return;

    //Update animation time
    animTime += Kernel::GetInstance()->getInterval() * playSpeed;
    
    //Check if animation has ended
    if (animTime > maxTime)
    {
      //Check if loop limit reached
      if (numLoops < maxLoops)
      {
        //Wrap animation time
        if (maxTime > 0.0f) {
          while (animTime > maxTime)
            animTime -= maxTime;
        } else animTime = 0.0;

        //Increase number of loops
        numLoops++;
      }
      else
      {
        //Clip animation time and stop
        animTime = maxTime;
        playing = false;
        paused = false;

        //Invoke callback
        if (endFunc != NULL)
          endFunc( this, endParam );
      }
    }
    
    //Evaluate animation
    evaluateAnimation();
  }

  void AnimController::evaluateAnimation ()
  {
    //Must have an animation bound
    if (anim == NULL) return;
    
    //Find 2 keys and interpolation coeff
    Float curKey = animTime * anim->kps;
    key1 = (Int) FLOOR( curKey );
    key2 = (Int) CEIL( curKey );
    keyT = curKey - (Float) key1;

    //Walk the list of tracks under current key
    for (TrackIter ti = tracksOnKey.begin(); ti != tracksOnKey.end(); )
    {
      //Update track values
      AnimTrack *track = anim->tracks[ *ti ];
      evaluateTrack( *ti );

      //Remove finished tracks
      if (key1 >= ( track->firstKey + track->getNumKeys() - 1 ))
        ti = tracksOnKey.removeAt( ti );
      else ++ti;
    }

    //Walk the list of pending animation tracks
    for (; trackIndex < anim->tracks.size(); ++trackIndex)
    {
      //Check if animation keys have been met at current time
      AnimTrack *track = anim->tracks[ trackIndex ];
      if (key1 >= track->firstKey)
      {
        //Evaluate animation and add to list
        evaluateTrack( (Int) trackIndex );
        tracksOnKey.pushBack( (Int) trackIndex );
      }
      else break;
    }

    //Walk the list of observers at current key
    for (UintSize o=0; o < observersOnKey.size(); ++o)
    {
      observersOnKey[ o ]->onAnyValueChanged();
      observersOnKey[ o ]->change = false;
    }

    //Clear the list of observers
    observersOnKey.clear();
  }

  void AnimController::evaluateTrack (UintSize t)
  {
    //Evaluate track at current time
    AnimTrack *track = anim->tracks[ t ];
    track->evalAt( key1, key2, keyT );

    //Get the observer of this track
    AnimObserver *observer = bindings[ t ]->observer;
    if (observer == NULL) return;

    //Notify of the value change
    observer->onValueChanged( track, bindings[ t ]->param );

    //Add to list of observers at current key
    if (observer->change == false)
    {
      observersOnKey.pushBack( observer );
      observer->change = true;
    }
  }

}//namespace GE
