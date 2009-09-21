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
#include "engine/embedit/shadenode.SkinMeshActor.embedded"

namespace GE
{

  DEFINE_SERIAL_CLASS( SkinMeshActor, ClassID( 0x208f56c3u, 0x4ce3, 0x4dee, 0xaafe9c235e6d42f3ull ));


  SkinMeshActor::SkinMeshActor()
  {
    character = NULL;
    skinVertices = NULL;
    skinNormals = NULL;
    boneRotations = NULL;
    boneTranslations = NULL;
  }

  SkinMeshActor::SkinMeshActor(SM *sm)
    : TriMeshActor(sm), character(sm)
  {
    skinVertices = NULL;
    skinNormals = NULL;
    boneRotations = NULL;
    boneTranslations = NULL;
  }

  void SkinMeshActor::onResourcesLoaded()
  {
    setCharacter( character );
  }

  SkinMeshActor::~SkinMeshActor()
  {
    freeAnimData();
  }

  void SkinMeshActor::setCharacter (const CharString &name)
  {
    character = name;
  }

  void SkinMeshActor::setCharacter (Character *c)
  {
    TriMeshActor::setMesh( c->meshes.first() );
    vertexBinding.init( c->meshes.first()->getFormat() );
    character = c;

    freeAnimData();
    initAnimData();
    loadPose();
  }

  Character* SkinMeshActor::getCharacter()
  {
    return character;
  }

  void SkinMeshActor::initAnimData()
  {
    if (character == NULL) return;

    /*
    //Init vertex and normal coordinates
    SkinVertex vert;
    SkinTriMesh *mesh = character->mesh;

    skinVertices = new Vector3[ mesh->data.size() ];
    skinNormals = new Vector3[ mesh->data.size() ];

    for (UintSize v=0; v < mesh->data.size(); ++v)
    {
      vert = vertexBinding( mesh->getVertex( v ) );
      skinVertices[ v ] = *vert.coord;
      skinNormals[ v ] = *vert.normal;
    }
    */

    //Init bone rotations and translations
    boneRotations = new Quat[ character->pose->joints.size() ];
    boneTranslations = new Vector3[ character->pose->joints.size() ];
  }

  void SkinMeshActor::freeAnimData()
  {
    if (skinVertices != NULL) {
      delete [] skinVertices;
      skinVertices = NULL;
    }

    if (skinNormals != NULL) {
      delete [] skinNormals;
      skinNormals = NULL;
    }

    if (boneRotations != NULL) {
      delete [] boneRotations;
      boneRotations = NULL;
    }

    if (boneTranslations != NULL) {
      delete [] boneTranslations;
      boneTranslations = NULL;
    }
  }

  void SkinMeshActor::loadPoseRotations()
  {
    for (UintSize b=0; b < character->pose->joints.size(); ++b) {
      boneRotations[ b ] = character->pose->joints[ b ].localR;
      boneTranslations[ b ] = character->pose->joints[ b ].localT;
    }
  }

  void SkinMeshActor::loadAnimRotations()
  {
    Float animTime = animCtrl.getTime();

    for (UintSize b=0; b < anim->tracksR.size(); ++b)
      boneRotations[ b ] = anim->tracksR[ b ]->evalAt( animTime );

    for (UintSize b=0; b < anim->tracksT.size(); ++b)
      boneTranslations[ b ] = anim->tracksT[ b ]->evalAt( animTime );
  }

  void SkinMeshActor::applySkin ()
  {
    SkinPose *pose = character->pose;
    ArrayList <Matrix4x4> fkMats;
    skinMats.clear();
    int cindex = 1;
    
    //Root FK matrix = local matrix
    Matrix4x4 rootWorld;
    rootWorld.fromQuat( boneRotations[0] );
    rootWorld.setColumn( 3, boneTranslations[0].xyz(1.0f) );
    rootWorld *= pose->joints.first().localS;
    fkMats.pushBack( rootWorld );
    
    //Walk all the joints
    for (UintSize p=0; p<pose->joints.size(); ++p)
    {
      //Final skin matrix = FK matrix * world matrix inverse
      SkinJoint *parent = &pose->joints[p];
      skinMats.pushBack( fkMats[p] * parent->worldInv );
      
      //Walk the children
      for (Uint32 c=0; c<parent->numChildren; ++c)
      {
        //Child FK matrix = parent FK matrix * local matrix
        SkinJoint *child = &pose->joints[ cindex ];

        Matrix4x4 childLocal;
        childLocal.fromQuat( boneRotations[ cindex ]);
        childLocal.setColumn( 3, boneTranslations[ cindex ].xyz(1.0f) );
        childLocal *= child->localS;
        fkMats.pushBack( fkMats[p] * childLocal );
        cindex++;
      }
    }

    /*
    //Apply rotations to vertices
    SkinTriMesh *mesh = character->mesh;
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

    anim = NULL;
    animCtrl.stop();
    loadPoseRotations();
    applySkin();
  }

  void SkinMeshActor::loadAnimation (const CharString &name)
  {
    if (character == NULL) return;
    
    SkinAnim *newAnim = character->findAnimByName( name );
    if (newAnim == NULL) return;

    anim = newAnim;
    animCtrl.stop();
    animCtrl.setLength( anim->duration );
    loadAnimRotations();
    applySkin();
  }

  void SkinMeshActor::tick ()
  {
    animCtrl.tick();
    if (animCtrl.isPlaying())
    {
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
    shader->composeNodeCode( skinCoordNode );
    shader->composeNodeEnd();

    //This node applies skin to normal
    shader->composeNodeNew( ShaderType::Vertex );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Normal );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Normal );
    shader->composeNodeCode( skinNormalNode );
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
