#include "engine/actors/geSkinMeshActor.h"
#include "engine/geCharacter.h"
#include "engine/geSkinPose.h"
#include "engine/geSkinMesh.h"
#include "engine/geSkinAnim.h"
#include "engine/geKernel.h"

namespace GE
{

  DEFINE_CLASS( SkinMeshActor );


  SkinMeshActor::SkinMeshActor()
  {
    character = NULL;
    poseVertices = NULL;
    poseNormals = NULL;
    boneRotations = NULL;
  }

  SkinMeshActor::~SkinMeshActor()
  {
    freeAnimData();
  }

  void SkinMeshActor::setMesh (MaxCharacter *c)
  {
    TriMeshActor::setMesh( (GE::TriMesh*) c->mesh );
    character = c;

    freeAnimData();
    initAnimData();
    loadPoseRotations();
  }

  MaxCharacter* SkinMeshActor::getMesh()
  {
    return character;
  }

  void SkinMeshActor::freeAnimData()
  {
    if (poseVertices != NULL)
      delete [] poseVertices;
    if (poseNormals != NULL)
      delete [] poseNormals;
    if (boneRotations != NULL)
      delete [] boneRotations;
  }

  void SkinMeshActor::initAnimData()
  {
    if (character == NULL) return;

    SkinTriMesh *mesh = character->mesh;

    poseVertices = new Vector3[ mesh->data.size() ];
    poseNormals = new Vector3[ mesh->data.size() ];
    boneRotations = new Quat[ character->pose->bones.size() ];

    for (UintSize v=0; v < mesh->data.size(); ++v)
    {
      poseVertices[ v ] = mesh->getVertex( v )->point;
      poseNormals[ v ] = mesh->getVertex( v )->normal;
    }

    anim = NULL;
    animSpeed = 1.0f;
    animIsPlaying = false;
    animIsLooping = false;
    animIsPaused = false;
  }

  void SkinMeshActor::loadPoseRotations()
  {
    for (UintSize b=0; b < character->pose->bones.size(); ++b)
      boneRotations[ b ] = character->pose->bones[ b ].localR;
  }

  void SkinMeshActor::loadAnimRotations()
  {
    for (UintSize b=0; b < character->pose->bones.size(); ++b)
      boneRotations[ b ] = anim->tracks[ b ]->evalAt( animTime );
    //boneRotations[ b ] = anim->tracks[ b ]->keys->at( frame ).value;
  }

  void SkinMeshActor::applySkin ()
  {
    SkinTriMesh *mesh = character->mesh;
    SkinPose *pose = character->pose;
    SkinAnim *anim = character->anims.first();
    ArrayList <Matrix4x4> fkMats;
    ArrayList <Matrix4x4> skinMats;
    int cindex = 1;
    
    //Root FK matrix = local matrix
    Matrix4x4 rootWorld;
    rootWorld.fromQuat( boneRotations[0] );
    rootWorld.setColumn( 3, pose->bones.first().localT );
    rootWorld *= pose->bones.first().localS;
    fkMats.pushBack( rootWorld );
    
    //Walk all the bones
    for (UintSize b=0; b<pose->bones.size(); ++b)
    {
      //Final skin matrix = FK matrix * world matrix inverse
      SkinBone *parent = &pose->bones[b];
      skinMats.pushBack( fkMats[b] * parent->worldInv );
      
      //Walk the children
      for (Uint32 c=0; c<parent->numChildren; ++c)
      {
        //Child FK matrix = parent FK matrix * local matrix
        SkinBone *child = &pose->bones[ cindex ];
        
        Matrix4x4 childLocal;
        childLocal.fromQuat( boneRotations[ cindex ]);
        childLocal.setColumn( 3, child->localT );
        childLocal *= child->localS;
        fkMats.pushBack( fkMats[b] * childLocal );
        cindex++;
      }
    }

    //Apply rotations to vertices

    int vindex = 0; int nindex = 0;
    
    for (UintSize index=0; index<mesh->data.size(); ++index)
    {
      SkinTriMesh::Vertex &v = *mesh->getVertex( index );
      
      v.point.set( 0,0,0 );
      for (int i=0; i<4; ++i)
      {
        Vector3 &posePoint = poseVertices[ index ];
        Vector3 skinPoint = skinMats[ v.boneIndex[i] ] * posePoint;
        v.point += skinPoint * v.boneWeight[i];
      }
      
      v.normal.set( 0,0,0 );
      for (int i=0; i<4; ++i)
      {
        Vector3 &poseNormal = poseNormals[ index ];
        Vector3 skinNormal = skinMats[ v.boneIndex[i] ].transformVector( poseNormal );
        v.normal += skinNormal * v.boneWeight[i];
      }
    }
  }

  void SkinMeshActor::playAnimation (const CharString &name, Float speed)
  {
    if (character == NULL) return;
    SkinAnim *newAnim = character->findAnimByName( name );
    if (newAnim == NULL) return;
    
    anim = newAnim;
    animStart = Kernel::GetInstance()->getTime();
    animTime = 0.0f;
    animSpeed = speed;
    animIsPlaying = true;
    animIsLooping = false;
  }

  void SkinMeshActor::loopAnimation (const CharString &name, Float speed)
  {
    if (character == NULL) return;
    SkinAnim *newAnim = character->findAnimByName( name );
    if (newAnim == NULL) return;
    
    anim = newAnim;
    animStart = Kernel::GetInstance()->getTime();
    animTime = 0.0f;
    animSpeed = speed;
    animIsPlaying = true;
    animIsLooping = true;
    animIsPaused = false;
  }

  void SkinMeshActor::stopAnimation ()
  {
    anim = NULL;
    animIsPlaying = false;
    animIsLooping = false;
    animIsPaused = false;
  }

  void SkinMeshActor::pauseAnimation ()
  {
    if (animIsPlaying)
      animIsPaused = !animIsPaused;
  }

  void SkinMeshActor::setAnimationSpeed (Float speed) {
    animSpeed = speed;
  }

  Float SkinMeshActor::getAnimationSpeed () {
    return animSpeed;
  }

  bool SkinMeshActor::isAnimationPlaying () {
    return animIsPlaying;
  }

  bool SkinMeshActor::isAnimationPaused () {
    return animIsPaused;
  }

  void SkinMeshActor::tick ()
  {
    if (animIsPlaying && !animIsPaused)
    {
      //Update animation time
      Float totalTime = anim->tracks.first()->totalTime;
      animTime += Kernel::GetInstance()->getInterval() * animSpeed;
      
      //Check if animation has ended
      if (animTime > totalTime)
      {
        if (animIsLooping)
        {
          //Wrap the animation time if looping
          while (animTime > totalTime)
            animTime -= totalTime;
        }
        else
        {
          //Clip and stop non-looping animation
          animTime = totalTime;
          animIsPlaying = false;
        }
      }

      //Update model
      loadAnimRotations();
      applySkin();
    }
  }

}//namespace GE
