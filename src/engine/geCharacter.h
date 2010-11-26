#ifndef __GECHARACTER_H
#define __GECHARACTER_H

#include "util/geUtil.h"
#include "engine/geResource.h"

namespace GE
{
  /*
  ----------------------------------------
  Forward declarations
  ----------------------------------------*/

  class SkinMesh;
  class SkinPose;
  class SkinTriMesh;
  class Animation;

  /*
  ----------------------------------------
  Forward declarations
  ----------------------------------------*/

  class Character : public Resource
  {
    CLASS( Character, Resource,
      5182b6a3,df16,4fe6,9d7f372e33a5dcce );

    virtual void serialize (Serializer *s, Uint v)
    {
      Resource::serialize( s, v );
      s->objectPtr( &pose );
      s->objectPtrArray( &meshes );
      s->objectPtrArray( &anims );
    }

  public:
    
    SkinPose *pose;
    ArrayList <SkinTriMesh*> meshes;
    ArrayList <Animation*> anims;
    
    Character ();
    ~Character ();

    Animation* findAnimByName (const CharString &name);
  };


}//namespace GE
#endif//__GECHARACTER_H
