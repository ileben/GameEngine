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
  class SkinAnim;
  class SkinTriMesh;

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
    ObjPtrArrayList <SkinAnim> anims;
    
    Character (SM *sm) : Resource(sm), meshes(sm), anims(sm) {}
    Character ();
    ~Character ();

    SkinAnim* findAnimByName (const CharString &name);
  };


}//namespace GE
#endif//__GECHARACTER_H
