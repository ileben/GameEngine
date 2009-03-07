#include "engine/actors/geSkinMeshActor.h"
#include "engine/geCharacter.h"
#include "engine/geSkinPose.h"
#include "engine/geSkinMesh.h"

namespace GE
{

  SkinMeshActor::SkinMeshActor()
  {
    character = NULL;
    poseVertices = NULL;
    poseNormals = NULL;
    boneRotations = NULL;
  }

  SkinMeshActor::~SkinMeshActor()
  {
    if (poseVertices != NULL)
      delete [] poseVertices;
    if (poseNormals != NULL)
      delete [] poseNormals;
    if (boneRotations != NULL)
      delete [] boneRotations;
  }

  void SkinMeshActor::setMesh (MaxCharacter *mesh)
  {
    character = mesh;
    TriMeshActor::setMesh( (GE::TriMesh*) character->trimesh );
  }

  MaxCharacter* SkinMeshActor::getMesh()
  {
    return character;
  }

}//namespace GE
