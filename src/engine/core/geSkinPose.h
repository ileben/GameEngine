#ifndef __GESKELETON_RES_H
#define __GESKELETON_RES_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "math/geMatrix.h"

namespace GE
{
  class SkinJoint : public Object
  {
    CLASS( SkinJoint, Object,
      7662f9e6,93c7,4c70,a7b3b6d516583c73 );

    virtual void serialize( Serializer *s, Uint v )
    {
      Object::serialize( s,v );
      s->string( &name );
      s->data( &numChildren );
      s->data( &worldInv );
      s->data( &localR );
      s->data( &localT );
      s->data( &localS );
    }

  public:

    CharString name;
    Uint32     numChildren;
    Matrix4x4  worldInv;
    Quat       localR;
    Vector3    localT;
    Matrix4x4  localS;
  };
  
  class SkinPose : public Object
  {
    CLASS( SkinPose, Object,
      14c0b93b,c4af,43d7,8056eb512f9be69e );

    virtual void serialize( Serializer *s, Uint v )
    {
      Object::serialize( s,v );
      s->objectArray( &joints );
    }
    
  public:

    ArrayList <SkinJoint> joints;
  };
  
}//namespace GE
#endif//__GESKELETON_RES_H
