#include "geTriMeshActor.h"
#include "engine/geTriMesh.h"
#include "engine/geGLHeaders.h"
#include "engine/geShader.h"
#include "engine/geKernel.h"
#include "engine/geRenderer.h"

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

  void TriMeshActor::beginVertexData (Shader *shader, const VFormat &format, void *data)
  {
    for (UintSize m=0; m<format.members.size(); ++m)
    {
      VFMember &member = format.members[m];
      void *memberData = Util::PtrOff( data, member.offset );

      GLenum gltype;
      switch (member.type) {
      case VFType::Float: gltype = GL_FLOAT; break;
      case VFType::Int:   gltype = GL_INT; break;
      case VFType::Uint:  gltype = GL_UNSIGNED_INT; break;
      }

      GLboolean glnormalize = (member.attribNorm ? GL_TRUE : GL_FALSE);

      switch (member.target)
      {
      case VFTarget::Coord:
        glVertexPointer( member.count, gltype, (GLsizei) format.size, memberData);
        glEnableClientState( GL_VERTEX_ARRAY );
        break;
      case VFTarget::TexCoord:
        glTexCoordPointer( member.count, gltype, (GLsizei) format.size, memberData);
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        break;
      case VFTarget::Normal:
        glNormalPointer( gltype, (GLsizei) format.size, memberData );
        glEnableClientState( GL_NORMAL_ARRAY );
        break;
      case VFTarget::Attribute:
        if (shader == NULL) break;
        Int32 glattrib = shader->getVertexAttribID( member.attribID );
        glVertexAttribPointer( glattrib, member.count, gltype, glnormalize, (GLsizei) format.size, memberData );
        glEnableVertexAttribArray( glattrib );
        break;
      }
    }
  }

  void TriMeshActor::endVertexData (Shader *shader, const VFormat &format)
  {
    for (UintSize m=0; m<format.members.size(); ++m)
    {
      VFMember &member = format.members[m];
      switch (member.target)
      {
      case VFTarget::Coord:
        glDisableClientState( GL_VERTEX_ARRAY );
        break;
      case VFTarget::TexCoord:
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        break;
      case VFTarget::Normal:
        glDisableClientState( GL_NORMAL_ARRAY );
        break;
      case VFTarget::Attribute:
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
    TriMesh::VertexFormat format;
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
}
