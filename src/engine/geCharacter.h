#ifndef __GECHARACTER_H
#define __GECHARACTER_H

#include "util/geUtil.h"

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

  class GE_API_ENTRY MaxCharacter
  {
    DECLARE_SERIAL_CLASS (MaxCharacter)
    DECLARE_OBJPTR( pose );
    DECLARE_OBJPTR( mesh );
    DECLARE_OBJVAR( anims );
    DECLARE_END;

  public:
    
    SkinPose *pose;
    SkinTriMesh *mesh;
    ArrayList<SkinTriMesh*> meshes;
    ObjPtrArrayList <SkinAnim> anims;
    
    MaxCharacter (SM *sm) : anims (sm) {}
    MaxCharacter ();
    ~MaxCharacter ();

    SkinAnim* findAnimByName (const CharString &name);
  };


}//namespace GE
#endif//__GECHARACTER_H
