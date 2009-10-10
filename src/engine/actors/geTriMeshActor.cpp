#include "geTriMeshActor.h"
#include "engine/geTriMesh.h"
#include "engine/geGLHeaders.h"
#include "engine/geShader.h"
#include "engine/geKernel.h"
#include "engine/geRenderer.h"
#include "engine/geShader.h"

namespace GE
{
  DEFINE_SERIAL_CLASS (TriMeshActor, ClassID( 0x94263b78u, 0xc1e9, 0x4e4a, 0x9f89fd667eadc891ull ));
  
  TriMeshActor::TriMeshActor()
  {
    mesh = NULL;
  }

  TriMeshActor::~TriMeshActor()
  {
    if (mesh != NULL)
      mesh->dereference();
  }

  void TriMeshActor::setMesh (const CharString &name) {
    mesh = name;
  }

  void TriMeshActor::setMesh (TriMesh *newMesh)
  {
    if (mesh != NULL)
      mesh->dereference();
    
    mesh = newMesh;
    mesh->reference();
  }

  TriMesh* TriMeshActor::getMesh()
  {
    return mesh;
  }

  BoundingBox TriMeshActor::getBoundingBox()
  {
    if (mesh == NULL) return BoundingBox();
    return mesh->getBoundingBox();
  }

  void TriMeshActor::composeShader( Shader *shader )
  {
    if (mesh == NULL) return;

    //Get mesh vertex format
    const VertexFormat *format = mesh->getFormat();
    attributeIDs.clear();

    //Walk the vertex data members
    for (UintSize m=0; m<format->getMembers()->size(); ++m)
    {
      //Check if data type is an attribute
      FormatMember &member= format->getMembers()->at( m );
      if (member.data == ShaderData::Attribute)
      {
        //Register all attribute members with the shader
        Int32 id = shader->registerVertexAttrib( member.attribUnit, member.attribName );
        attributeIDs.pushBack( id );
      }
      else attributeIDs.pushBack( -1 );
    }
  }

  void TriMeshActor::bindFormat (Shader *shader, VertexFormat *format)
  {
    //Get data pointer
    void *data = NULL;
    if (!mesh->isOnGpu)
      data = mesh->data.buffer();
    
    //Walk the vertx data members
    for (UintSize m=0; m<format->getMembers()->size(); ++m)
    {
      //Get the offset into the data
      FormatMember &member= format->getMembers()->at( m );
      void *memberData = Util::PtrOff( data, member.offset );
      UintSize stride = format->getByteSize();

      //Find the GL data type
      GLenum gltype;
      switch (member.unit.type) {
      case DataType::Float: gltype = GL_FLOAT; break;
      case DataType::Int:   gltype = GL_INT; break;
      case DataType::Uint:  gltype = GL_UNSIGNED_INT; break;
      }

      //Check if attribute is to be normalized
      GLboolean glnormalize = (member.attribNorm ? GL_TRUE : GL_FALSE);
      Int32 attIndex = attributeIDs[ m ];

      //Pass the data pointer to OpenGL
      switch (member.data)
      {
      case ShaderData::Coord:
        glVertexPointer( member.unit.count, gltype, (GLsizei) stride, memberData);
        glEnableClientState( GL_VERTEX_ARRAY );
        break;
      case ShaderData::TexCoord:
        glTexCoordPointer( member.unit.count, gltype, (GLsizei) stride, memberData);
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        break;
      case ShaderData::Normal:
        glNormalPointer( gltype, (GLsizei) stride, memberData );
        glEnableClientState( GL_NORMAL_ARRAY );
        break;
      case ShaderData::Attribute:
        if (shader == NULL) break;
        //Int32 glattrib = shader->getVertexAttribID( attributeIDs[ m ] );
        Int32 glattrib = shader->getVertexAttribID( member.attribName );
        if (glattrib == -1) break;
        glVertexAttribPointer( glattrib, member.unit.count, gltype, glnormalize, (GLsizei) stride, memberData );
        glEnableVertexAttribArray( glattrib );
        break;
      }
    }
  }

  void TriMeshActor::unbindFormat (Shader *shader, VertexFormat *format)
  {
    for (UintSize m=0; m<format->getMembers()->size(); ++m)
    {
      FormatMember &member= format->getMembers()->at( m );
      switch (member.data)
      {
      case ShaderData::Coord:
        glDisableClientState( GL_VERTEX_ARRAY );
        break;
      case ShaderData::TexCoord:
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        break;
      case ShaderData::Normal:
        glDisableClientState( GL_NORMAL_ARRAY );
        break;
      case ShaderData::Attribute:
        if (shader == NULL) break;
        //Int32 glattrib = shader->getVertexAttribID( attributeIDs[ m ] );
        Int32 glattrib = shader->getVertexAttribID( member.attribName );
        if (glattrib == -1) break;
        glDisableVertexAttribArray( glattrib );
        break;
      }
    }
  }

  void TriMeshActor::bindBuffers()
  {
    if (mesh->isOnGpu)
    {
      //Render using on-GPU arrays
      glBindBuffer( GL_ARRAY_BUFFER, mesh->dataVBO );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->indexVBO );
    }
  }

  void TriMeshActor::unbindBuffers()
  {
    if (mesh->isOnGpu)
    {
      //Restore arrays
      glBindBuffer( GL_ARRAY_BUFFER, 0 );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    }
  }

  void TriMeshActor::renderGroup (const TriMesh::IndexGroup &grp)
  {
    //Pass the geometry to OpenGL
    if (mesh->isOnGpu) {

      //Render using on-GPU indices
      glDrawElements( GL_TRIANGLES, grp.count, GL_UNSIGNED_INT,
                      Util::PtrOff( 0, grp.start * sizeof(VertexID)) );
    }else{

      //Render using off-GPU indices
      glDrawElements( GL_TRIANGLES, grp.count, GL_UNSIGNED_INT,
                      mesh->indices.buffer() + grp.start);
    }
  }

  void TriMeshActor::renderShadow ()
  {
    VertexFormat *format = (VertexFormat*) mesh->getFormat();
    Renderer *renderer = Kernel::GetInstance()->getRenderer();
    Shader *shader = renderer->getShader( RenderTarget::ShadowMap, this, NULL );

    shader->use();
    bindBuffers();
    bindFormat( shader, format );

    //Walk material index groups
    for (UintSize g=0; g<mesh->groups.size(); ++g)
    {
      //Render current group
      TriMesh::IndexGroup &grp = mesh->groups[ g ];
      renderGroup( grp );
    }

    Material::EndDefault();
    unbindFormat( shader, format );
    unbindBuffers();
  }

  void TriMeshActor::renderSingleMat ()
  {
    Material *material = getMaterial();
    VertexFormat *format = (VertexFormat*) mesh->getFormat();
    Renderer *renderer = Kernel::GetInstance()->getRenderer();
    Shader *shader = renderer->getShader( RenderTarget::GBuffer, this, material );

    shader->use();
    bindBuffers();
    bindFormat( shader, format );
    material->begin();

    //Walk material index groups
    for (UintSize g=0; g<mesh->groups.size(); ++g)
    {
      //Render current group
      TriMesh::IndexGroup &grp = mesh->groups[ g ];
      renderGroup( grp );
    }

    material->end();
    unbindFormat( shader, format );
    unbindBuffers();
  }

  void TriMeshActor::renderMultiMat ()
  {
    Material *material = getMaterial();
    VertexFormat *format = (VertexFormat*) mesh->getFormat();
    MultiMaterial *multiMat = (MultiMaterial*) material;
    Renderer *renderer = Kernel::GetInstance()->getRenderer();
    Shader *shader = NULL;

    bindBuffers();

    //Walk material index groups
    for (UintSize g=0; g<mesh->groups.size(); ++g)
    {
      //Get sub-material of this group
      TriMesh::IndexGroup &grp = mesh->groups[ g ];
      Material *subMat = multiMat->getSubMaterial( grp.materialID );
      if (subMat == NULL) continue;

      //Find shader for this material
      Shader *subShader = renderer->getShader( RenderTarget::GBuffer, this, subMat );
      if (subShader != shader)
      {
        //If different, resend format data
        shader = subShader;
        shader->use();
        bindFormat( shader, format );
      }
      
      //Render current group
      subMat->begin();
      renderGroup( grp );
      subMat->end();
    }
    
    unbindFormat( shader, format );
    unbindBuffers();
  }

  void TriMeshActor::render (RenderTarget::Enum target)
  {
    Material *material = getMaterial();
    if (mesh == NULL) return;
    if (material == NULL) return;
    if (target == RenderTarget::ShadowMap)
    {
      renderShadow();
    }
    else if (target == RenderTarget::GBuffer)
    {
      MultiMaterial *multiMat = SafeCast( MultiMaterial, material );
      if (multiMat == NULL) renderSingleMat();
      else renderMultiMat();
    }
  }

}//namespace GE
