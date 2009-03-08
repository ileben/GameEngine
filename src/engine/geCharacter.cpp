#include "engine/geCharacter.h"
#include "engine/geSkinMesh.h"
#include "engine/geSkinPose.h"
#include "engine/geSkinAnim.h"

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

  SkinAnim* MaxCharacter::findAnimByName (const CharString &name)
  {
    for (UintSize a=0; a < anims.size(); ++a)
      if (anims[ a ]->name == name)
        return anims[ a ];

    return NULL;
  }

}//namespace GE
