#include "geCharacter.h"
#include "geSkinMesh.h"
#include "geSkinPose.h"
#include "geSkinAnim.h"

namespace GE
{
  
  DEFINE_SERIAL_CLASS (MaxCharacter, CLSID_MAXCHARACTER);

  MaxCharacter::MaxCharacter()
  {
    mesh = NULL;
    pose = NULL;
  }

  MaxCharacter::~MaxCharacter ()
  {
    if (mesh != NULL)
      delete mesh;

    if (pose != NULL)
      delete pose;
    
    for (UintSize a=0; a<anims.size(); ++a)
      delete anims[ a ];
  }

}//namespace GE
