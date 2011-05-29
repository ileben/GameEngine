#include "core/actors/geSkinMeshActor.h"
#include "core/geCharacter.h"
#include "core/geSkinPose.h"
#include "core/geSkinMesh.h"
#include "core/geSkinAnim.h"
#include "core/geKernel.h"
#include "core/geGLHeaders.h"
#include "core/geShader.h"
#include "core/geShaders.h"
#include "core/geRenderer.h"
#include "core/embedit/shadenode.SkinMeshActor.embedded"

namespace GE
{

  SkinMeshActor::SkinMeshActor()
  {
    character = NULL;
    skinVertices = NULL;
    skinNormals = NULL;
    jointRotations = NULL;
    jointTranslations = NULL;
    jointChange = false;
    numJoints = 0;
  }

  void SkinMeshActor::onResourcesLoaded()
  {
    if (character != NULL)
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

    //Init BBox by first mesh
    if (!c->meshes.empty())
      charBBox = c->meshes.first()->getBoundingBox();

    //Merge with the rest of the meshes
    for (UintSize m=1; m < c->meshes.size(); ++m)
      charBBox += c->meshes[m]->getBoundingBox();
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

    //Init joint rotations and translations
    numJoints = character->pose->joints.size();
    jointRotations = new Quat[ numJoints ];
    jointTranslations = new Vector3[ numJoints ];
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

    if (jointRotations != NULL) {
      delete [] jointRotations;
      jointRotations = NULL;
    }

    if (jointTranslations != NULL) {
      delete [] jointTranslations;
      jointTranslations = NULL;
    }
  }

  int SkinMeshActor::getJointIndex (const CharString &jointName)
  {
    if (character == NULL)
      return -1;

    for (UintSize j=0; j<character->pose->joints.size(); ++j)
      if (character->pose->joints[ j ].name == jointName)
        return (int) j;

    return -1;
  }

  void SkinMeshActor::setJointRotation (int jointIndex, Quat r)
  {
    //Set rotation to given value
    if (jointIndex < (int) character->pose->joints.size())
      jointRotations[ jointIndex ] = r;

    //Mark changes
    jointChange = true;
  }

  void SkinMeshActor::setJointTranslation (int jointIndex, Vector3 t)
  {
    //Set translation to given value
    if (jointIndex < (int) character->pose->joints.size())
      jointTranslations[ jointIndex ] = t;

    //Mark changes
    jointChange = true;
  }

  void SkinMeshActor::setPoseRotations()
  {
    //Load transformations from character pose
    for (UintSize j=0; j < character->pose->joints.size(); ++j)
    {
      jointRotations[ j ] = character->pose->joints[ j ].localR;
      jointTranslations[ j ] = character->pose->joints[ j ].localT;
    }

    //Mark changes
    jointChange = true;
  }

  void SkinMeshActor::setAnimRotations()
  {
    //Load transformations from animation tracks
    for (UintSize j=0; j < character->pose->joints.size(); ++j)
    {
      QuatAnimTrack *trackR = (QuatAnimTrack*) anim->getTrack( 2 * j + 0 );
      Vec3AnimTrack *trackT = (Vec3AnimTrack*) anim->getTrack( 2 * j + 1 );

      jointRotations[ j ] = trackR->getValue();
      jointTranslations[ j ] = trackT->getValue();
    }

    //Mark changes
    jointChange = true;
  }

  void SkinMeshActor::updateSkin ()
  {
    if (character == NULL) return;

    //Check if any change in joint transformations
    if (!jointChange) return;

    SkinPose *pose = character->pose;
    ArrayList <Matrix4x4> fkMats;
    skinMats.clear();
    int cindex = 1;
    
    //Root FK matrix = local matrix
    Matrix4x4 rootWorld;
    rootWorld.fromQuat( jointRotations[0] );
    rootWorld.setColumn( 3, jointTranslations[0].xyz(1.0f) );
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
        childLocal.fromQuat( jointRotations[ cindex ]);
        childLocal.setColumn( 3, jointTranslations[ cindex ].xyz(1.0f) );
        childLocal *= child->localS;
        fkMats.pushBack( fkMats[p] * childLocal );
        cindex++;
      }
    }

    //Unmark changes
    jointChange = false;

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
    setPoseRotations();
    updateSkin();
  }

  void SkinMeshActor::loadAnimation (const CharString &name)
  {
    if (character == NULL) return;
    
    Animation *newAnim = character->findAnimByName( name );
    if (newAnim == NULL) return;

    anim = newAnim;
    animCtrl.bindAnimation( anim );
    animCtrl.observeAt( 0.0f );
    setAnimRotations();
    updateSkin();
  }

  void SkinMeshActor::tick ()
  {
    animCtrl.tick();
    if (animCtrl.isPlaying())
    {
      //Update model
      setAnimRotations();
      updateSkin();
    }
  }

  void SkinMeshActor::composeShader( Shader *shader )
  {
    TriMeshActor::composeShader( shader );

    //Register the uniform to pass matrix data in
    skinMatUniform = shader->registerUniform( ShaderType::Vertex, DataUnit::Mat4, "skinMatrix", 24 );

    //This node applies skin to the vertex coordinate
    shader->composeNodeNew( ShaderType::Vertex );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Coord3 );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Coord3 );
    shader->composeNodeCode( skinCoordNode );
    shader->composeNodeEnd();

    //This node applies skin to normal
    shader->composeNodeNew( ShaderType::Vertex );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Normal );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Normal );
    shader->composeNodeCode( skinNormalNode );
    shader->composeNodeEnd();

    //This node applies skin to tangent
    shader->composeNodeNew( ShaderType::Vertex );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Tangent );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Tangent );
    shader->composeNodeCode( skinTangentNode );
    shader->composeNodeEnd();

    //This node applies skin to bitanget
    shader->composeNodeNew( ShaderType::Vertex );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Bitangent );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Bitangent );
    shader->composeNodeCode( skinBitangentNode );
    shader->composeNodeEnd();
  }

  void SkinMeshActor::bindFormat (Shader *shader, VertexFormat *format)
  {
    TriMeshActor::bindFormat (shader, format);

    //Construct a mesh-specific array of joint matrices
    Matrix4x4 meshMats[24];
    for (UintSize b=0; b<curSubMesh->mesh2skinSize; ++b)
      meshMats[b] = skinMats[ curSubMesh->mesh2skinMap[b] ];

    GLenum err = glGetError();

    //Pass joint matrices to the shader
    Int32 uniMatrix = shader->getUniformID( skinMatUniform );
    glUniformMatrix4fv( uniMatrix, curSubMesh->mesh2skinSize, GL_FALSE, (GLfloat*)meshMats );

    err = glGetError();
  }

  void SkinMeshActor::render (RenderTarget::Enum target)
  {
    //Make sure there's something to render
    if (character == NULL) return;

    //Make sure skin is up to date
    updateSkin();

    //Walk sub meshes
    for (UintSize m=0; m<character->meshes.size(); ++m)
    {
      //Render this sub mesh
      curSubMesh = character->meshes[m];
      this->mesh = curSubMesh;
      TriMeshActor::render( target );
    }
  }

  BoundingBox SkinMeshActor::getBoundingBox()
  {
    Matrix4x4 rootWorld;

    //Find root joint world matrix
    if (character != NULL && numJoints > 0) {
      rootWorld.fromQuat( jointRotations[0] );
      rootWorld.setColumn( 3, jointTranslations[0].xyz(1.0f) );
      rootWorld *= character->pose->joints.first().localS;
    }

    //Transform by root joint matrix
    BoundingBox outBBox = charBBox;
    outBBox.min = rootWorld * outBBox.min;
    outBBox.max = rootWorld * outBBox.max;

    return outBBox;
  }

}//namespace GE
