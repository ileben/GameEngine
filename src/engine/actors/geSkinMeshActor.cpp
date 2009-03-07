#include "engine/actors/geSkinMeshActor.h"
#include "engine/geCharacter.h"
#include "engine/geSkinPose.h"
#include "engine/geSkinMesh.h"
#include "engine/geSkinAnim.h"

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
    TriMeshActor::setMesh( (GE::TriMesh*) c->trimesh );
    character = c;

    freeAnimData();
    initAnimData();
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

    SkinTriMesh *mesh = character->trimesh;

    poseVertices = new Vector3[ mesh->data.size() ];
    poseNormals = new Vector3[ mesh->data.size() ];
    boneRotations = new Quat[ character->pose->bones.size() ];

    for (UintSize v=0; v < mesh->data.size(); ++v)
    {
      poseVertices[ v ] = mesh->getVertex( v )->point;
      poseNormals[ v ] = mesh->getVertex( v )->normal;
    }
  }

  void SkinMeshActor::animate (Float curTime)
  {
    SkinTriMesh *mesh = character->trimesh;
    SkinPose *pose = character->pose;
    SkinAnim *anim = character->anims.first();
    ArrayList <Matrix4x4> fkMats;
    ArrayList <Matrix4x4> skinMats;
    int cindex = 1;
    
    UintSize numTracks = anim->tracks.size();
    UintSize numKeys = anim->tracks.first()->keys.size();
    
    //Root FK matrix = local matrix
    Matrix4x4 rootWorld;
    //rootWorld.fromQuat( anim->tracks->first()->keys->at( frame ).value);
    rootWorld.fromQuat( anim->tracks.first()->evalAt( curTime ));
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
        SkinTrack *track = anim->tracks[ cindex ];
        cindex++;
        
        Matrix4x4 childLocal;
        //childLocal.fromQuat( track->keys->at( frame ).value);
        childLocal.fromQuat( track->evalAt( curTime ));
        childLocal.setColumn( 3, child->localT );
        childLocal *= child->localS;
        fkMats.pushBack( fkMats[b] * childLocal );
      }
    }

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

}//namespace GE
