#include "engine/geCharacter.h"
#include "engine/geSkinMesh.h"
#include "engine/geSkinPose.h"
#include "engine/geSkinAnim.h"

namespace GE
{
  
  DEFINE_SERIAL_CLASS( Character, ClassID (0xc0db7169u, 0x65dd, 0x4375, 0xa4b2d9a505703db8ull ));

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
