#include "engine/actors/geSkinMeshActor.h"
#include "engine/geCharacter.h"
#include "engine/geSkinPose.h"
#include "engine/geSkinMesh.h"
#include "engine/geSkinAnim.h"
#include "engine/geKernel.h"
#include "engine/geGLHeaders.h"
#include "engine/geShader.h"
#include "engine/geShaders.h"
#include "engine/geRenderer.h"

namespace GE
{

  DEFINE_CLASS( SkinMeshActor );


  SkinMeshActor::SkinMeshActor()
  {
    character = NULL;
    skinVertices = NULL;
    skinNormals = NULL;
    boneRotations = NULL;
    boneTranslations = NULL;
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
    applySkin();
  }

  MaxCharacter* SkinMeshActor::getMesh()
  {
    return character;
  }

  void SkinMeshActor::freeAnimData()
  {
    if (skinVertices != NULL)
      delete [] skinVertices;
    if (skinNormals != NULL)
      delete [] skinNormals;
    if (boneRotations != NULL)
      delete [] boneRotations;
    if (boneTranslations != NULL)
      delete [] boneTranslations;
  }

  void SkinMeshActor::initAnimData()
  {
    if (character == NULL) return;

    SkinTriMesh *mesh = character->mesh;

    skinVertices = new Vector3[ mesh->data.size() ];
    skinNormals = new Vector3[ mesh->data.size() ];
    boneRotations = new Quat[ character->pose->bones.size() ];
    boneTranslations = new Vector3[ character->pose->bones.size() ];

    for (UintSize v=0; v < mesh->data.size(); ++v)
    {
      skinVertices[ v ] = mesh->getVertex( v )->point;
      skinNormals[ v ] = mesh->getVertex( v )->normal;
    }

    anim = NULL;
    animSpeed = 1.0f;
    animIsPlaying = false;
    animIsLooping = false;
    animIsPaused = false;
  }

  void SkinMeshActor::loadPoseRotations()
  {
    for (UintSize b=0; b < character->pose->bones.size(); ++b) {
      boneRotations[ b ] = character->pose->bones[ b ].localR;
      boneTranslations[ b ] = character->pose->bones[ b ].localT;
    }
  }

  void SkinMeshActor::loadAnimRotations()
  {
    for (UintSize b=0; b < anim->tracksR.size(); ++b)
      boneRotations[ b ] = anim->tracksR[ b ]->evalAt( animTime );

    for (UintSize b=0; b < anim->tracksT.size(); ++b)
      boneTranslations[ b ] = anim->tracksT[ b ]->evalAt( animTime );
  }

  void SkinMeshActor::applySkin ()
  {
    //SkinTriMesh *mesh = character->mesh;
    SkinPose *pose = character->pose;
    SkinAnim *anim = character->anims.first();
    ArrayList <Matrix4x4> fkMats;
    skinMats.clear();
    int cindex = 1;
    
    //Root FK matrix = local matrix
    Matrix4x4 rootWorld;
    rootWorld.fromQuat( boneRotations[0] );
    rootWorld.setColumn( 3, boneTranslations[0].xyz(1.0f) );
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
        childLocal.setColumn( 3, boneTranslations[ cindex ].xyz(1.0f) );
        childLocal *= child->localS;
        fkMats.pushBack( fkMats[b] * childLocal );
        cindex++;
      }
    }

    //Apply rotations to vertices
    /*
    int vindex = 0; int nindex = 0;
    
    for (UintSize index=0; index<mesh->data.size(); ++index)
    {
      SkinTriMesh::Vertex &v = *mesh->getVertex( index );
      
      skinVertices[ index ].set( 0,0,0 );
      for (int i=0; i<4; ++i)
      {
        Vector3 skinPoint = skinMats[ v.boneIndex[i] ] * v.point;
        skinVertices[ index ] += skinPoint * v.boneWeight[i];
      }
      
      skinNormals[ index ].set( 0,0,0 );
      for (int i=0; i<4; ++i)
      {
        Vector3 skinNormal = skinMats[ v.boneIndex[i] ].transformVector( v.normal );
        skinNormals[ index ] += skinNormal * v.boneWeight[i];
      }
    }*/

    /*
    SkinTriMesh *mesh = character->meshes.first();

    //Construct a mesh-specific array of skin matrices
    Matrix4x4 meshMats[24];
    for (UintSize b=0; b<mesh->mesh2skinSize; ++b)
      meshMats[b] = skinMats[ mesh->mesh2skinMap[b] ];

    int vindex = 0; int nindex = 0;
    
    for (UintSize index=0; index<mesh->getVertexCount(); ++index)
    {
      SkinTriMesh::Vertex &v = *mesh->getVertex( index );
      
      skinVertices[ index ].set( 0,0,0 );
      for (int i=0; i<4; ++i)
      {
        Vector3 skinPoint = meshMats[ v.boneIndex[i] ] * v.point;
        skinVertices[ index ] += skinPoint * v.boneWeight[i];
      }
      
      skinNormals[ index ].set( 0,0,0 );
      for (int i=0; i<4; ++i)
      {
        Vector3 skinNormal = meshMats[ v.boneIndex[i] ].transformVector( v.normal );
        skinNormals[ index ] += skinNormal * v.boneWeight[i];
      }
    }*/
  }

  void SkinMeshActor::loadPose ()
  {
    if (character == NULL) return;

    stopAnimation();
    loadPoseRotations();
    applySkin();
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
      animTime += Kernel::GetInstance()->getInterval() * animSpeed;
      
      //Check if animation has ended
      if (animTime > anim->duration)
      {
        if (animIsLooping)
        {
          //Wrap the animation time if looping
          while (animTime > anim->duration)
            animTime -= anim->duration;
        }
        else
        {
          //Clip and stop non-looping animation
          animTime = anim->duration;
          animIsPlaying = false;
        }
      }

      //Update model
      loadAnimRotations();
      applySkin();
    }
  }

  void SkinMeshActor::composeShader( Shader *shader )
  {
    TriMeshActor::composeShader( shader );

    //Register the uniform to pass matrix data in
    skinMatUniform = shader->registerUniform( ShaderType::Vertex, DataUnit::Mat4, "skinMatrix", 24 );

    //This node applies skin to the vertex coordinate
    shader->composeNodeNew( ShaderType::Vertex );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Coord );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Coord );
    shader->composeNodeCode(
      "outCoord = vec4( 0,0,0,0 );\n"
      "for (int i=0; i<4; ++i)\n"
      "  outCoord += boneWeight[i] * (skinMatrix[ int(boneIndex[i]) ] * inCoord);\n" );
    shader->composeNodeEnd();

    //This node applies skin to normal
    shader->composeNodeNew( ShaderType::Vertex );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Normal );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Normal );
    shader->composeNodeCode(
      "vec4 inNormal4 = vec4( inNormal, 0.0 );\n"
      "outNormal = vec3( 0,0,0 );\n"
      "for (int i=0; i<4; ++i)\n"
      "  outNormal += boneWeight[i] * (skinMatrix[ int(boneIndex[i]) ] * inNormal4).xyz;\n" );
    shader->composeNodeEnd();
  }

  void SkinMeshActor::render (MaterialID materialID)
  {
    //Make sure there's something to render
    if (character == NULL) return;
    Shader *shader = Kernel::GetInstance()->getRenderer()->getCurrentShader();

    //Walk sub meshes
    for (UintSize m=0; m<character->meshes.size(); ++m)
    {
      SkinTriMesh *subMesh = character->meshes[m];

      //Construct a mesh-specific array of skin matrices
      Matrix4x4 meshMats[24];
      for (UintSize b=0; b<subMesh->mesh2skinSize; ++b)
        meshMats[b] = skinMats[ subMesh->mesh2skinMap[b] ];

      //Pass bone matrices to the shader
      if (shader != NULL) {
        Int32 uniMatrix = shader->getUniformID( skinMatUniform );
        glUniformMatrix4fv( uniMatrix, subMesh->mesh2skinSize, GL_FALSE, (GLfloat*)meshMats );
      }
      
      //Render this sub mesh
      this->mesh = subMesh;
      TriMeshActor::render( materialID );
    }
  }

}//namespace GE
