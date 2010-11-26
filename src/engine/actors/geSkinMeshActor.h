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
  class SkinTriMesh;
  class SkinMeshActor;

  /*
  ------------------------------------------------
  This actor animates and renders a skinned mesh
  ------------------------------------------------*/

  class SkinMeshActor : public TriMeshActor
  {
    CLASS( SkinMeshActor, TriMeshActor,
      ab4bef78,7eba,4391,a8c93ede5d34c641 );

    virtual void serialize( Serializer *s, Uint v ) {
      TriMeshActor::serialize( s,v );
      s->object( &character );
    }

  private:
    BoundingBox charBBox;
    CharacterRef character;
    UintSize numJoints;
    Vector3 *skinVertices;
    Vector3 *skinNormals;
    Vector3 *jointTranslations;
    Quat    *jointRotations;
    ArrayList <Matrix4x4> skinMats;
    VertexBinding <SkinVertex> vertexBinding;
    bool jointChange;

    void freeAnimData();
    void initAnimData();
    void setPoseRotations();
    void setAnimRotations();
    void updateSkin();

    Animation *anim;
    AnimController animCtrl;

    Int32 skinMatUniform;
    Int32 boneIndexAttrib;
    Int32 boneWeightAttrib;
    SkinTriMesh *curSubMesh;
    virtual void bindFormat (Shader *shader, VertexFormat *format);

  public:
    virtual Class getShaderComposingClass() { return ClassName( SkinMeshActor ); }
    virtual void composeShader( Shader *shader );

    SkinMeshActor ();
    virtual void onResourcesLoaded();
    virtual ~SkinMeshActor ();

    void setCharacter (const CharString &name);
    void setCharacter (Character *mesh);
    Character* getCharacter ();

    int getJointIndex (const CharString &jointName);
    void setJointRotation (int jointIndex, Quat rotation);
    void setJointTranslation (int jointIndex, Vector3 rotation);

    virtual void render (RenderTarget::Enum target);

    void loadPose ();
    void loadAnimation (const CharString &name);
    AnimController *getAnimController () { return &animCtrl; }
    void tick ();

    virtual BoundingBox getBoundingBox();
  };


}//namespace GE
#endif//__GESKINMESHACTOR_H
