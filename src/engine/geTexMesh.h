#ifndef __GEUVMESH_H
#define __GEUVMESH_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "geHmesh.h"

namespace GE
{

  class TexMeshTraits : public HMesh
  {
  public:

    class Vertex : public VertexBase <TexMeshTraits,HMesh> {
      CLASS( TexMeshTraits::Vertex, c6e741d5,5105,4831,aa53a87962500bfb );
    public:
      Vector2 point;
    };

    class HalfEdge : public HalfEdgeBase <TexMeshTraits,HMesh> {
      CLASS( TexMeshTraits::HalfEdge, c16493f7,ece6,4dba,ae0783d0f3ba7b55 );
    };
    
    class Edge : public EdgeBase <TexMeshTraits,HMesh> {
      CLASS( TexMeshTraits::Edge, ffe54ba4,62a7,4b98,9c7610a3b0f78708 );
    };
    
    class Face : public FaceBase <TexMeshTraits,HMesh> {
      CLASS( TexMeshTraits::Face, 0a57e38a,1e88,4c51,9ec02cc86e789ee4 );
    };
  };

  class TexMesh : public MeshBase <TexMeshTraits,HMesh>
  {
    CLASS( TexMesh, 21674b6b,1ac5,441b,8800656097790a96 );
  };

}/* namespace GE */

#endif
