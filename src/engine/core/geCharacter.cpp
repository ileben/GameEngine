#include "core/geCharacter.h"
#include "core/geSkinMesh.h"
#include "core/geSkinPose.h"
#include "core/geSkinAnim.h"

namespace GE
{

  Character::Character()
  {
    pose = NULL;
  }

  Character::~Character ()
  {
    if (pose != NULL)
      delete pose;

    for (UintSize m=0; m<meshes.size(); ++m)
      delete meshes[ m ];

    for (UintSize a=0; a<anims.size(); ++a)
      delete anims[ a ];
  }

  Animation* Character::findAnimByName (const CharString &name)
  {
    for (UintSize a=0; a < anims.size(); ++a)
      if (anims[ a ]->name == name)
        return anims[ a ];

    return NULL;
  }

}//namespace GE
