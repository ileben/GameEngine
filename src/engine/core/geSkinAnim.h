#ifndef __GESKELANIM_H
#define __GESKELANIM_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "core/geAnimation.h"
#include "core/geActor.h"
#include "core/geCamera.h"
#include "core/actors/geSkinMeshActor.h"

namespace GE
{

  class ActorAnimObserver : public AnimObserver
  {
    CLASS( ActorAnimObserver, AnimObserver,
      d03e90b5,be96,41c8,beb31606ad9ec84f );

    virtual void serialize( Serializer *s, Uint v )
    {
      AnimObserver::serialize( s,v );
      s->data( &valueS );
      s->objectRef( &actor );
    }

  public:

    Matrix4x4 valueS;
    Vector3 valueT;
    Quat    valueR;
    Actor3D *actor;
    ActorAnimObserver () : actor (NULL) {}
    
    virtual void onValueChanged (AnimTrack *track, Int param)
    {
      if (ClassOf( track ) == ClassName( Vec3AnimTrack ))
        valueT = ((Vec3AnimTrack*) track)->getValue();
      
      else if (ClassOf( track ) == ClassName( QuatAnimTrack ))
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
    CLASS( SkinAnimObserver, AnimObserver,
      89d4c1bb,9d12,481c,9329d0f78a6e4d01 );

    virtual void serialize( Serializer *s, Uint v )
    {
      AnimObserver::serialize( s,v );
      s->objectRef( &actor );
    }

  public:

    SkinMeshActor *actor;
    SkinAnimObserver () : actor (NULL) {}

    virtual void onValueChanged (AnimTrack *track, Int param)
    {
      if (ClassOf( track ) == ClassName( Vec3AnimTrack ))
      {
        Vec3AnimTrack* vtrack = (Vec3AnimTrack*) track;
        actor->setJointTranslation( param, vtrack->getValue() );
      }
      else if (ClassOf( track ) == ClassName( QuatAnimTrack ))
      {
        QuatAnimTrack* qtrack = (QuatAnimTrack*) track;
        actor->setJointRotation( param, qtrack->getValue() );
      }
    }
  };

  class DofAnimObserver : public AnimObserver
  {
    CLASS( DofAnimObserver, AnimObserver,
      ef012909,a311,43a8,b7632dabff49bc37 );

    virtual void serialize( Serializer *s, Uint v )
    {
      AnimObserver::serialize( s,v );
      s->objectRef( &camera );
    }

  public:

    Camera3D *camera;
    DofAnimObserver () : camera (NULL) {}

    virtual void onValueChanged (AnimTrack *track, Int param)
    {
      if (ClassOf( track ) == ClassName( FloatAnimTrack ))
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
      else if (ClassOf( track ) == ClassName( BoolAnimTrack ))
      {
        BoolAnimTrack *btrack = (BoolAnimTrack*) track;
        //camera->setDofEnabled( btrack->getValue() );
      }
    }
  };

}//namespace GE
#endif//__GESKELANIM_H
