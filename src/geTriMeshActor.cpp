#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"
using namespace OCC;

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

  void TriMeshActor::render (MaterialId materialId)
  {
    //Draw our mesh
    if (mesh != NULL)
      renderMesh (materialId);
  }

  void TriMeshActor::renderMesh (MaterialId materialId)
  {
    //Walk material index groups
    for (LinkedList<TriMesh::IndexGroup>::Iterator
         g = mesh->groups.begin ();
         g != mesh->groups.end (); ++g)
    {
      //Check if the material id matches
      if (g->materialId == materialId || materialId == GE_ANY_MATERIAL_ID)
      {
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        
        glInterleavedArrays (GL_T2F_N3F_V3F, 0, mesh->data.buffer());
        glDrawElements (GL_TRIANGLES, g->count, GL_UNSIGNED_INT,
                        mesh->indices.buffer() + g->start);
        
        glDisableClientState (GL_VERTEX_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);
        glDisableClientState (GL_TEXTURE_COORD_ARRAY);
        
        break;
      }
    }
  }
}
