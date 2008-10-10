#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"

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

  void TriMeshActor::render (MaterialID materialID)
  {
    //Draw our mesh
    if (mesh != NULL)
      renderMesh( materialID );
  }
  
  void TriMeshActor::renderMesh (MaterialID materialID)
  {
    //Walk material index groups
    for (UintSize g=0; g<mesh->groups->size(); ++g)
    {
      //Check if the material id matches
      TriMesh::IndexGroup &grp = mesh->groups->at( g );
      if (materialID != grp.materialID &&
          materialID != GE_ANY_MATERIAL_ID)
        continue;
      
      //Pass the geometry to OpenGL
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      
      glInterleavedArrays( GL_T2F_N3F_V3F,
                           (GLsizei) mesh->data->elementSize(),
                           mesh->data->buffer());
      
      glDrawElements( GL_TRIANGLES, grp.count, GL_UNSIGNED_INT,
                      mesh->indices->buffer() + grp.start);
      
      glDisableClientState( GL_VERTEX_ARRAY );
      glDisableClientState( GL_NORMAL_ARRAY );
      glDisableClientState( GL_TEXTURE_COORD_ARRAY );
      
      break;
    }
  }
}
