#ifndef __GEUVMESH_H
#define __GEUVMESH_H

namespace GE
{

  class GE_API_ENTRY TexMesh : public HMesh
  {
    DECLARE_SUBCLASS (TexMesh, HMesh); DECLARE_END;
  
  public:

    class GE_API_ENTRY Vertex : public VertexBase <TexMesh,HMesh> {
      DECLARE_SUBCLASS (Vertex, HMesh::Vertex); DECLARE_END;
    public:
      Vector2 point;
    };

    class GE_API_ENTRY HalfEdge : public HalfEdgeBase <TexMesh,HMesh> {
      DECLARE_SUBCLASS (HalfEdge, HMesh::HalfEdge); DECLARE_END;
    };
    
    class GE_API_ENTRY Edge : public EdgeBase <TexMesh,HMesh> {
      DECLARE_SUBCLASS (Edge, HMesh::Edge); DECLARE_END;
    };
    
    class GE_API_ENTRY Face : public FaceBase <TexMesh,HMesh> {
      DECLARE_SUBCLASS (Face, HMesh::Face); DECLARE_END;
    };
    
    #include "geHmeshDataiter.h"
    #include "geHmeshAdjiter.h"
    
    TexMesh() {
      setClasses(
        Class(Vertex),
        Class(HalfEdge),
        Class(Edge),
        Class(Face));
    }
  };

}/* namespace GE */

#endif
