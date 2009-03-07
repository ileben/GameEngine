#ifndef __GESKINMESHACTOR_H
#define __GESKINMESHACTOR_H

#include "util/geUtil.h"
#include "engine/actors/geTriMeshActor.h"

namespace GE
{

  /*
  ------------------------------------
  Forward declarations
  ------------------------------------*/

  class MaxCharacter;

  /*
  ------------------------------------------------
  This actor animates and renders a skinned mesh
  ------------------------------------------------*/

  class SkinMeshActor : public TriMeshActor
  {
    DECLARE_SUBCLASS( SkinMeshActor, TriMeshActor );
    DECLARE_END;

  private:
    MaxCharacter *character;
    Vector3 *poseVertices;
    Vector3 *poseNormals;
    Quat *boneRotations;

    void freeAnimData();
    void initAnimData();

  public:
    SkinMeshActor();
    virtual ~SkinMeshActor();

    void setMesh (MaxCharacter *mesh);
    MaxCharacter* getMesh();

    void animate (Float time);
  };


}//namespace GE
#endif//__GESKINMESHACTOR_H
