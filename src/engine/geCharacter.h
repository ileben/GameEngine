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
    DECLARE_SERIAL_SUBCLASS( Character, Resource )
    DECLARE_OBJPTR( pose );
    DECLARE_OBJVAR( meshes );
    DECLARE_OBJVAR( anims );
    DECLARE_END;

  public:
    
    SkinPose *pose;
    ObjPtrArrayList <SkinTriMesh> meshes;
    ObjPtrArrayList <Animation> anims;
    
    Character (SM *sm) : Resource(sm), meshes(sm), anims(sm) {}
    Character ();
    ~Character ();

    Animation* findAnimByName (const CharString &name);
  };

  /*
  class Character : public Resource
  {
  public:

    UUID( 0xc0db7169u, 0x65dd, 0x4375, 0xa4b2d9a505703db8ull );

    virtual void serialize (Serializer *s, Uint v)
    {
      Resource::serialize( s, v );
      s->objectPtr( pose );
      s->objectPtrArray( meshes );
      s->objectPtrArray( anims );
    }

  public:

    SkinPose *pose;
    ArrayList <SkinTriMesh> meshes;
    ArrayList <Animation> anims;
    
    Character ();
    ~Character ();

    Animation* findAnimByName (const CharString &name);
  };
*/

}//namespace GE
#endif//__GECHARACTER_H
