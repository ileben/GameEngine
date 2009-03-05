#ifndef __GEUVMESH_H
#define __GEUVMESH_H

#include "util/geUtil.h"
#include "geHmesh.h"
#include "geVectors.h"

namespace GE
{

  class GE_API_ENTRY TexMeshTraits : public HMesh
  {
  public:

    class GE_API_ENTRY Vertex : public VertexBase <TexMeshTraits,HMesh> {
      DECLARE_SUBCLASS (Vertex, HMesh::Vertex); DECLARE_END;
    public:
      Vector2 point;
    };

    class GE_API_ENTRY HalfEdge : public HalfEdgeBase <TexMeshTraits,HMesh> {
      DECLARE_SUBCLASS (HalfEdge, HMesh::HalfEdge); DECLARE_END;
    };
    
    class GE_API_ENTRY Edge : public EdgeBase <TexMeshTraits,HMesh> {
      DECLARE_SUBCLASS (Edge, HMesh::Edge); DECLARE_END;
    };
    
    class GE_API_ENTRY Face : public FaceBase <TexMeshTraits,HMesh> {
      DECLARE_SUBCLASS (Face, HMesh::Face); DECLARE_END;
    };
  };

  class TexMesh : public MeshBase <TexMeshTraits,HMesh>
  {
    DECLARE_SUBCLASS (TexMesh, HMesh); DECLARE_END;
  };

}/* namespace GE */

#endif
