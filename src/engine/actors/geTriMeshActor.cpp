#include "geTriMeshActor.h"
#include "engine/geTriMesh.h"
#include "engine/geGLHeaders.h"
#include "engine/geShader.h"
#include "engine/geKernel.h"
#include "engine/geRenderer.h"
#include "engine/geShader.h"

namespace GE
{

  TriMeshActor::TriMeshActor()
  {
    mesh = NULL;
    meshVAO = 0;
    meshVAOInit = false;
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

    //Walk the vertex data members
    for (UintSize m=0; m<format->getMembers()->size(); ++m)
    {
      //Register attributes with the shader
      FormatMember &member= format->getMembers()->at( m );
      Int32 id = shader->registerVertexAttrib( member.attribUnit, member.attribName );
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

  void TriMeshActor::bindFormat (Shader *shader, VertexFormat *format)
  {
    if (shader == NULL)
      return;

    //Get data pointer
    void *data = NULL;
    if (!mesh->isOnGpu)
      data = mesh->data.buffer();
    
    //Walk the vertex data members
    for (UintSize m=0; m<format->getMembers()->size(); ++m)
    {
      //Get the offset into the data
      FormatMember &member= format->getMembers()->at( m );
      void *memberData = Util::PtrOff( data, member.offset );
      UintSize stride = format->getByteSize();

      //Find the input data type
      GLenum gltype;
      switch (member.unit.type) {
      case DataType::Float: gltype = GL_FLOAT; break;
      case DataType::Int:   gltype = GL_INT; break;
      case DataType::Uint:  gltype = GL_UNSIGNED_INT; break;
      }

      //Check if attribute is to be normalized
      GLboolean glnormalize = (member.attribNorm ? GL_TRUE : GL_FALSE);

      //Get the vertex attribute index
      Int32 glattrib = shader->getVertexAttribID( m );
      if (glattrib == -1) continue;
      
      //Enable attribute and set its data pointer
      glEnableVertexAttribArray( glattrib );
      glVertexAttribPointer( glattrib, member.unit.count, gltype, glnormalize, (GLsizei) stride, memberData );
    }
  }

  void TriMeshActor::unbindFormat (Shader *shader, VertexFormat *format)
  {
    if (shader == NULL)
      return;

    //Walk the vertex data members
    for (UintSize m=0; m<format->getMembers()->size(); ++m)
    {
      FormatMember &member= format->getMembers()->at( m );

      //Get the vertex attribute index
      Int32 glattrib = shader->getVertexAttribID( m );
      if (glattrib == -1) continue;
      
      //Disable attribute
      glDisableVertexAttribArray( glattrib );
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

  void TriMeshActor::renderShadowSingle ()
  {
    Material *material = getMaterial();
    VertexFormat *format = (VertexFormat*) mesh->getFormat();
    Renderer *renderer = Kernel::GetInstance()->getRenderer();
    Shader *shader = renderer->getShader( RenderTarget::ShadowMap, this, NULL );

    shader->use();
    bindBuffers();
    bindFormat( shader, format );

    material->beginShadow();

    //Walk material index groups
    for (UintSize g=0; g<mesh->groups.size(); ++g)
    {
      //Render current group
      TriMesh::IndexGroup &grp = mesh->groups[ g ];
      renderGroup( grp );
    }

    material->endShadow();

    unbindFormat( shader, format );
  }

  void TriMeshActor::renderShadowMulti()
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
      Shader *subShader = renderer->getShader( RenderTarget::ShadowMap, this, subMat );
      if (subShader != shader)
      {
        //If different, resend format data
        shader = subShader;
        shader->use();
        bindFormat( shader, format );
      }
      
      //Render current group
      subMat->beginShadow();
      renderGroup( grp );
      subMat->endShadow();
    }
    
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

#if (0)

    if (!meshVAOInit)
    {
      glGenVertexArrays( 1, &meshVAO );
      glBindVertexArray( meshVAO );
      bindBuffers();
      bindFormat( shader, format );
      meshVAOInit = true;
    }
    else
      glBindVertexArray( meshVAO );

#else

    bindBuffers();
    bindFormat( shader, format );

#endif

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
      MultiMaterial *multiMat = Class::SafeCast< MultiMaterial >( material );
      if (multiMat == NULL) renderShadowSingle();
      else renderShadowMulti();
    }
    else if (target == RenderTarget::GBuffer)
    {
      MultiMaterial *multiMat = Class::SafeCast< MultiMaterial >( material );
      if (multiMat == NULL) renderSingleMat();
      else renderMultiMat();
    }
  }

}//namespace GE
