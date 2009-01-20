#ifndef __GESKELETON_RES_H
#define __GESKELETON_RES_H

#include "util/geUtil.h"
#include "geVectors.h"
#include "geMatrix.h"

namespace GE
{
  struct GE_API_ENTRY SkinBone
  {
    Uint32     numChildren;
    Matrix4x4  worldInv;
    Quat       localR;
    Vector4    localT;
    Matrix4x4  localS;
  };
  
  class GE_API_ENTRY SkinPose
  {
    DECLARE_SERIAL_CLASS( SkinPose );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;
    
  public:
    ArrayList <SkinBone> bones;
    
    void serialize (void *sm) { ((SM*)sm)->objectVar( &bones ); }
    SkinPose (SM *sm) : bones (sm) {}
    SkinPose () {}
  };
  
}//namespace GE
#endif//__GESKELETON_RES_H
