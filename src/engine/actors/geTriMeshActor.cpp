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
    VFormat *format = mesh->getVertexFormat();

    //Walk the vertex data members
    for (UintSize m=0; m<format->members.size(); ++m)
    {
      //Register all attribute members with the shader
      VFMember &member= format->members[m];
      if (member.data != ShaderData::Attribute) continue;
      member.attribID = shader->registerVertexAttrib( member.attribUnit, member.attribName );
    }
  }

  void TriMeshActor::beginVertexData (Shader *shader, VFormat *format, void *data)
  {
    //Walk the vertx data members
    for (UintSize m=0; m<format->members.size(); ++m)
    {
      //Get the offset into the data
      VFMember &member = format->members[m];
      void *memberData = Util::PtrOff( data, member.offset );

      //Find the GL data type
      GLenum gltype;
      switch (member.unit.type) {
      case DataType::Float: gltype = GL_FLOAT; break;
      case DataType::Int:   gltype = GL_INT; break;
      case DataType::Uint:  gltype = GL_UNSIGNED_INT; break;
      }

      //Check if attribute is to be normalized
      GLboolean glnormalize = (member.attribNorm ? GL_TRUE : GL_FALSE);

      //Pass the data pointer to OpenGL
      switch (member.data)
      {
      case ShaderData::Coord:
        glVertexPointer( member.unit.count, gltype, (GLsizei) format->size, memberData);
        glEnableClientState( GL_VERTEX_ARRAY );
        break;
      case ShaderData::TexCoord:
        glTexCoordPointer( member.unit.count, gltype, (GLsizei) format->size, memberData);
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        break;
      case ShaderData::Normal:
        glNormalPointer( gltype, (GLsizei) format->size, memberData );
        glEnableClientState( GL_NORMAL_ARRAY );
        break;
      case ShaderData::Attribute:
        if (shader == NULL) break;
        Int32 glattrib = shader->getVertexAttribID( member.attribID );
        glVertexAttribPointer( glattrib, member.unit.count, gltype, glnormalize, (GLsizei) format->size, memberData );
        glEnableVertexAttribArray( glattrib );
        break;
      }
    }
  }

  void TriMeshActor::endVertexData (Shader *shader, VFormat *format)
  {
    for (UintSize m=0; m<format->members.size(); ++m)
    {
      VFMember &member = format->members[m];
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
        Int32 glattrib = shader->getVertexAttribID( member.attribID );
        glDisableVertexAttribArray( glattrib );
        break;
      }
    }
  }
  
  void TriMeshActor::render (MaterialID materialID)
  {
    //Make sure there's something to render
    if (mesh == NULL) return;
    VFormat *format = mesh->getVertexFormat();
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
