#ifndef __GESKELETON_RES_H
#define __GESKELETON_RES_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "math/geMatrix.h"

namespace GE
{
  class SkinJoint
  {
    DECLARE_SERIAL_CLASS( SkinJoint );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;

  public:

    CharString name;
    Uint32     numChildren;
    Matrix4x4  worldInv;
    Quat       localR;
    Vector3    localT;
    Matrix4x4  localS;

    SkinJoint (SM *sm) : name(sm) {}
    SkinJoint () {}
    
    void serialize (void *param)
    {
      SerializeManager *sm = (SM*)param;
      sm->objectVar( &name );
      sm->dataVar( &numChildren );
      sm->dataVar( &worldInv );
      sm->dataVar( &localR );
      sm->dataVar( &localT );
      sm->dataVar( &localS );
    }
  };
  
  class GE_API_ENTRY SkinPose
  {
    DECLARE_SERIAL_CLASS( SkinPose );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;
    
  public:

    ObjArrayList <SkinJoint> joints;
    
    SkinPose (SM *sm) : joints (sm) {}
    SkinPose () {}

    void serialize (void *sm) {
      ((SM*)sm)->objectVar( &joints);
    }
  };
  
}//namespace GE
#endif//__GESKELETON_RES_H
