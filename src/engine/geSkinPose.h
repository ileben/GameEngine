#ifndef __GESKELETON_RES_H
#define __GESKELETON_RES_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "math/geMatrix.h"

namespace GE
{
  class SkinJoint : public Object
  {
    DECLARE_SERIAL_SUBCLASS( SkinJoint, Object );
    DECLARE_OBJVAR( name );
    DECLARE_DATAVAR( numChildren );
    DECLARE_DATAVAR( worldInv );
    DECLARE_DATAVAR( localR );
    DECLARE_DATAVAR( localT );
    DECLARE_DATAVAR( localS );
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
  };
  
  class SkinPose : public Object
  {
    DECLARE_SERIAL_SUBCLASS( SkinPose, Object );
    DECLARE_OBJVAR( joints );
    DECLARE_END;
    
  public:

    ObjArrayList <SkinJoint> joints;
    
    SkinPose (SM *sm) : joints (sm) {}
    SkinPose () {}
  };
  
}//namespace GE
#endif//__GESKELETON_RES_H
