#ifndef __GESKELANIM_H
#define __GESKELANIM_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "engine/geAnimation.h"
#include "engine/geActor.h"

namespace GE
{

  class ActorAnimObserver : public AnimObserver
  {
    DECLARE_SERIAL_SUBCLASS( ActorAnimObserver, AnimObserver );
    DECLARE_OBJREF( actor );
    DECLARE_END;

  public:

    Vector3 valueT;
    Quat    valueR;
    Actor3D *actor;

    ActorAnimObserver () : actor (NULL) {};
    ActorAnimObserver (SM *sm) : AnimObserver (sm) {};
    
    virtual void onValueChanged (AnimTrack *track, Int param)
    {
      if (ClassOf( track ) == Class (Vec3AnimTrack ))
        valueT = ((Vec3AnimTrack*) track)->getValue();
      
      else if (ClassOf( track ) == Class (QuatAnimTrack))
        valueR = ((QuatAnimTrack*) track)->getValue();
    }

    virtual void onAnyValueChanged ()
    {
      Matrix4x4 mat;
      mat.fromQuat( valueR );
      mat.setColumn( 3, valueT );
      actor->setMatrix( mat );
    }
  };
/*
  class SkinAnimation : public Animation
  {
    DECLARE_SERIAL_SUBCLASS( SkinAnimation, Animation );
    DECLARE_END;

  public:

    SkinAnimation () {}
    SkinAnimation (SM *sm) : Animation (sm) {}

    void init
    {
    }
  };
*/

  /*
  class SkinAnimTrack : public AnimTrack
  {
    DECLARE_SERIAL_SUBCLASS( SkinAnimTrack, AnimTrack );
    DECLARE_OBJVAR( tracksT );
    DECLARE_OBJVAR( tracksR );
    DECLARE_END;
    
  public:

    ObjPtrArrayList <Vec3AnimTrack> tracksT;
    ObjPtrArrayList <QuatAnimTrack> tracksR;
    SkinMeshActor *actor;
    
    SkinAnimTrack (SM *sm) : AnimTrack (sm), tracksT (sm), tracksR(sm) {}
    SkinAnimTrack() {}
    ~SkinAnimTrack ()
    {
      for (UintSize t=0; t<tracksT.size(); ++t)
        delete tracksT[ t ];
      for (UintSize t=0; t<tracksR.size(); ++t)
        delete tracksR[ t ];
    }
    
    virtual void evalAt (Int key1, Int key2, Float keyT)
    {
      for (UintSize t=0; t<tracksT.size(); ++t)
      {
        tracksT[ t ]->evalAt( key1, key2, keyT );
        tracksR[ t ]->evalAt( key1, key2, keyT );

        //TODO: apply translation and rotation to bones
      }
    }
  };
  */
};

#endif//__GESKELANIM_H
