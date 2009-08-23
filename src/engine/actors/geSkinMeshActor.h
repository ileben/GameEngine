#ifndef __GESKINMESHACTOR_H
#define __GESKINMESHACTOR_H

#include "util/geUtil.h"
#include "engine/geSkinMesh.h"
#include "engine/actors/geTriMeshActor.h"

namespace GE
{

  /*
  ------------------------------------
  Forward declarations
  ------------------------------------*/

  class MaxCharacter;
  class SkinAnim;
  class SkinTriMesh;

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
    Vector3 *skinVertices;
    Vector3 *skinNormals;
    Vector3 *boneTranslations;
    Quat    *boneRotations;
    ArrayList <Matrix4x4> skinMats;
    VertexBinding <SkinVertex> vertexBinding;

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

    Int32 skinMatUniform;
    Int32 boneIndexAttrib;
    Int32 boneWeightAttrib;

  public:
    virtual ClassPtr getShaderComposingClass() { return Class(SkinMeshActor); }
    virtual void composeShader( Shader *shader );

    SkinMeshActor();
    virtual ~SkinMeshActor();

    void setMesh (MaxCharacter *mesh);
    MaxCharacter* getMesh();

    virtual void render (MaterialID materialID);

    void loadPose ();
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
