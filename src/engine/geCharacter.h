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

  /*
  ----------------------------------------
  Forward declarations
  ----------------------------------------*/

  class GE_API_ENTRY MaxCharacter
  {
    DECLARE_SERIAL_CLASS (MaxCharacter)
    DECLARE_CALLBACK (ClassEvent::Serialize, serialize);
    DECLARE_END;

  public:
    
    SkinMesh *mesh;
    SkinPose *pose;
    ClassArrayList <SkinAnim> anims;
    
    void serialize (void *sm)
    {
      if (mesh != NULL)
        ((SM*)sm)->objectPtr( &mesh );
      if (pose != NULL)
        ((SM*)sm)->objectPtr( &pose );
      
      ((SM*)sm)->objectVar( &anims );
    }
    
    MaxCharacter (SM *sm) : anims (sm) {}
    MaxCharacter ();
    ~MaxCharacter();
  };


}//namespace GE
#endif//__GECHARACTER_H
