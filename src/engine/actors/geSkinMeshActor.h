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
  class SkinAnim;

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
    void loadPoseRotations();
    void loadAnimRotations();
    void applySkin();

    SkinAnim *anim;
    Float animTime;
    Float animStart;
    Float animSpeed;
    bool animIsPlaying;
    bool animIsLooping;
    bool animIsPaused;

  public:
    SkinMeshActor();
    virtual ~SkinMeshActor();

    void setMesh (MaxCharacter *mesh);
    MaxCharacter* getMesh();

    void playAnimation (const CharString &name, Float speed=1.0f);
    void loopAnimation (const CharString &name, Float speed=1.0f);
    void pauseAnimation ();
    void stopAnimation ();
    void tick ();

    void setAnimationSpeed (Float speed);
    Float getAnimationSpeed ();
    bool isAnimationPlaying ();
    bool isAnimationPaused ();
  };


}//namespace GE
#endif//__GESKINMESHACTOR_H
