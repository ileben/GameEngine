#ifndef __GESKINMESHACTOR_H
#define __GESKINMESHACTOR_H

#include "util/geUtil.h"
#include "engine/geSkinMesh.h"
#include "engine/actors/geTriMeshActor.h"
#include "engine/geAnimation.h"

namespace GE
{

  /*
  ------------------------------------
  Forward declarations
  ------------------------------------*/

  class Character;
  class SkinAnim;
  class SkinTriMesh;
  class SkinMeshActor;

  /*
  ------------------------------------------------
  This actor animates and renders a skinned mesh
  ------------------------------------------------*/

  class SkinMeshActor : public TriMeshActor
  {
    DECLARE_SERIAL_SUBCLASS( SkinMeshActor, TriMeshActor );
    DECLARE_OBJVAR( character );
    DECLARE_END;

  private:
    CharacterRef character;
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
    AnimController animCtrl;

    Int32 skinMatUniform;
    Int32 boneIndexAttrib;
    Int32 boneWeightAttrib;

  public:
    virtual ClassPtr getShaderComposingClass() { return Class(SkinMeshActor); }
    virtual void composeShader( Shader *shader );

    SkinMeshActor ();
    SkinMeshActor (SM *sm);
    virtual void onResourcesLoaded();
    virtual ~SkinMeshActor ();

    void setCharacter (const CharString &name);
    void setCharacter (Character *mesh);
    Character* getCharacter ();

    virtual void render (MaterialID materialID);

    void loadPose ();
    void loadAnimation (const CharString &name);
    AnimController *getAnimController () { return &animCtrl; }
    void tick ();
  };


}//namespace GE
#endif//__GESKINMESHACTOR_H
