#ifndef __GESKELANIM_H
#define __GESKELANIM_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "engine/geAnimation.h"
#include "engine/geActor.h"
#include "engine/geCamera.h"
#include "engine/actors/geSkinMeshActor.h"

namespace GE
{

  class ActorAnimObserver : public AnimObserver
  {
    DECLARE_SERIAL_SUBCLASS( ActorAnimObserver, AnimObserver );
    DECLARE_DATAVAR( valueS );
    DECLARE_OBJREF( actor );
    DECLARE_END;

  public:

    Matrix4x4 valueS;
    Vector3 valueT;
    Quat    valueR;
    Actor3D *actor;

    ActorAnimObserver () : actor (NULL) {}
    ActorAnimObserver (SM *sm) : AnimObserver (sm) {}
    
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
      actor->setMatrix( mat * valueS );
    }
  };

  class SkinAnimObserver : public AnimObserver
  {
    DECLARE_SERIAL_SUBCLASS( SkinAnimObserver, AnimObserver );
    DECLARE_OBJREF( actor );
    DECLARE_END;

  public:

    SkinMeshActor *actor;

    SkinAnimObserver () : actor (NULL) {}
    SkinAnimObserver (SM *sm) : AnimObserver (sm) {}

    virtual void onValueChanged (AnimTrack *track, Int param)
    {
      if (ClassOf( track ) == Class( Vec3AnimTrack ))
      {
        Vec3AnimTrack* vtrack = (Vec3AnimTrack*) track;
        actor->setJointTranslation( param, vtrack->getValue() );
      }
      else if (ClassOf( track ) == Class (QuatAnimTrack ))
      {
        QuatAnimTrack* qtrack = (QuatAnimTrack*) track;
        actor->setJointRotation( param, qtrack->getValue() );
      }
    }
  };

  class DofAnimObserver : public AnimObserver
  {
    DECLARE_SERIAL_SUBCLASS( DofAnimObserver, AnimObserver );
    DECLARE_OBJREF( camera );
    DECLARE_END;

  public:

    Camera3D *camera;

    DofAnimObserver () : camera (NULL) {}
    DofAnimObserver (SM *sm) : AnimObserver (sm) {}

    virtual void onValueChanged (AnimTrack *track, Int param)
    {
      if (ClassOf( track ) == Class( FloatAnimTrack ))
      {
        DofParams dofParams = camera->getDofParams();
        FloatAnimTrack *ftrack = (FloatAnimTrack*) track;

        switch (param) {
        case 0: dofParams.focusCenter = ftrack->getValue(); break;
        case 1: dofParams.focusRange = ftrack->getValue(); break;
        case 2: dofParams.falloffNear = ftrack->getValue(); break;
        case 3: dofParams.falloffFar = ftrack->getValue(); break; }

        camera->setDofParams( dofParams );
      }
      else if (ClassOf( track ) == Class( BoolAnimTrack ))
      {
        BoolAnimTrack *btrack = (BoolAnimTrack*) track;
        camera->setDofEnabled( btrack->getValue() );
      }
    }
  };

}//namespace GE
#endif//__GESKELANIM_H
