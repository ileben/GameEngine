#include "geTriMeshActor.h"
#include "engine/geTriMesh.h"
#include "engine/geGLHeaders.h"
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

  void TriMeshActor::beginVertexData (Material *material, const VFormat &format, void *data)
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
        if (material == NULL) break;
        if (material->getShader() == NULL) break;
        Int32 glattrib = material->getShader()->getVertexAttribID( member.attribID );
        glVertexAttribPointer( glattrib, member.count, gltype, glnormalize, (GLsizei) format.size, memberData );
        glEnableVertexAttribArray( glattrib );
        break;
      }
    }
  }

  void TriMeshActor::endVertexData (Material *material, const VFormat &format)
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
        if (material == NULL) break;
        if (material->getShader() == NULL) break;
        Int32 glattrib = material->getShader()->getVertexAttribID( member.attribID );
        glDisableVertexAttribArray( glattrib );
        break;
      }
    }
  }
  
  void TriMeshActor::render (Material *material, MaterialID materialID)
  {
    //Make sure there's something to render
    if (mesh == NULL) return;
    TriMesh::VertexFormat format;

    //Walk material index groups
    for (UintSize g=0; g<mesh->groups.size(); ++g)
    {
      //Check if the material id matches
      TriMesh::IndexGroup &grp = mesh->groups[ g ];
      if (materialID != grp.materialID &&
          materialID != GE_ANY_MATERIAL_ID)
        continue;
      
      //Pass the geometry to OpenGL
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
/*
      if (mesh->isOnGpu)
      {
        GE_glBindBuffer( mesh->dataVBO, GL_ARRAY_BUFFER );
        GE_glBindBuffer( mesh->indexVBO, GL_ELEMENT_ARRAY_BUFFER );
        beginVertexData( material, format, NULL );
      }
      else*/ beginVertexData( material, format, mesh->data.buffer() );

      glBindBuffer( mesh->dataVBO, GL_ARRAY_BUFFER );

      glDrawElements( GL_TRIANGLES, grp.count, GL_UNSIGNED_INT,
                      mesh->indices.buffer() + grp.start);

      endVertexData( material, format );

      if (mesh->isOnGpu)
      {
        glBindBuffer( 0, GL_ARRAY_BUFFER );
        glBindBuffer( 0, GL_ELEMENT_ARRAY_BUFFER );
      }

      if (materialID != GE_ANY_MATERIAL_ID)
        break;
    }
  }
}
