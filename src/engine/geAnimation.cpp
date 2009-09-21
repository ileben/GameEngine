#include "engine/geAnimation.h"
#include "engine/geKernel.h"

namespace GE
{

  DEFINE_CLASS( IAnimTrack );
  DEFINE_SERIAL_TEMPL_CLASS( QuatAnimTrack,  CLSID_QUATANIMTRACK );
  DEFINE_SERIAL_TEMPL_CLASS( Vec3AnimTrack,  CLSID_VEC3ANIMTRACK );


  AnimController::AnimController()
  {
    maxTime = 0.0f;
    startTime = 0.0f;
    animTime = 0.0f;
    playSpeed = 1.0f;
    maxLoops = 0;
    numLoops = 0;
    endFunc = NULL;
  }

  void AnimController::play (Float length, Float from, Int nloops, Float speed)
  {
    maxTime = length;
    animTime = Util::Clamp( from, 0.0f, maxTime );
    maxLoops = nloops;
    playSpeed = speed;

    play();
  }

  void AnimController::play ()
  {
    startTime = Kernel::GetInstance()->getTime();
    playing = true;
    paused = false;
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

  void AnimController::stop ()
  {
    playing = false;
    paused = false;
    animTime = 0.0;
    numLoops = 0;
  }

  void AnimController::setLength (Float length)
  {
    maxTime = Util::Max( length, 0.0f );
    animTime = Util::Clamp( animTime, 0.0f, maxTime );
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
  }

}//namespace GE
