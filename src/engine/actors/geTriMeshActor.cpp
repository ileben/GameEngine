#include "geTriMeshActor.h"
#include "engine/geTriMesh.h"
#include "engine/geGLHeaders.h"
#include "engine/geShader.h"
#include "engine/geKernel.h"
#include "engine/geRenderer.h"
#include "engine/geShader.h"

namespace GE
{
  DEFINE_CLASS (TriMeshActor);
  
  TriMeshActor::TriMeshActor()
  {
    mesh = NULL;
  }

  TriMeshActor::~TriMeshActor()
  {
    if (mesh != NULL)
      mesh->dereference();
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

  void TriMeshActor::beginVertexData (Shader *shader, VertexFormat *format, void *data)
  {
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
        Int32 glattrib = shader->getVertexAttribID( attributeIDs[ m ] );
        if (glattrib == -1) break;
        glVertexAttribPointer( glattrib, member.unit.count, gltype, glnormalize, (GLsizei) stride, memberData );
        glEnableVertexAttribArray( glattrib );
        break;
      }
    }
  }

  void TriMeshActor::endVertexData (Shader *shader, VertexFormat *format)
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
        Int32 glattrib = shader->getVertexAttribID( attributeIDs[ m ] );
        if (glattrib == -1) break;
        glDisableVertexAttribArray( glattrib );
        break;
      }
    }
  }
  
  void TriMeshActor::render (MaterialID materialID)
  {
    //Make sure there's something to render
    if (mesh == NULL) return;
    VertexFormat *format = const_cast<VertexFormat*>( mesh->getFormat() );
    Shader *shader = Kernel::GetInstance()->getRenderer()->getCurrentShader();

    //Walk material index groups
    for (UintSize g=0; g<mesh->groups.size(); ++g)
    {
      //Check if the material id matches
      TriMesh::IndexGroup &grp = mesh->groups[ g ];
      if (materialID != grp.materialID &&
          materialID != GE_ANY_MATERIAL_ID)
        continue;
      
      //Pass the geometry to OpenGL

      if (mesh->isOnGpu)
      {
        GE_glBindBuffer( GL_ARRAY_BUFFER, mesh->dataVBO );
        GE_glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->indexVBO );
        beginVertexData( shader, format, NULL );
        glDrawElements( GL_TRIANGLES, grp.count, GL_UNSIGNED_INT,
                        Util::PtrOff( 0, grp.start * sizeof(VertexID)) );
      }
      else
      {
        beginVertexData( shader, format, mesh->data.buffer() );
        glDrawElements( GL_TRIANGLES, grp.count, GL_UNSIGNED_INT,
                        mesh->indices.buffer() + grp.start);
      }

      endVertexData( shader, format );

      if (mesh->isOnGpu)
      {
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
      }

      if (materialID != GE_ANY_MATERIAL_ID)
        break;
    }
  }

}//namespace GE
