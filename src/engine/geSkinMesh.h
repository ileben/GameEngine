#ifndef __GESKINPOLYMESH_H
#define __GESKINPOLYMESH_H

#include "util/geUtil.h"
#include "engine/geVectors.h"
#include "engine/geTriMesh.h"
#include "engine/gePolyMesh.h"

namespace GE
{

  /*
  ==========================================================
  SkinPolyMesh - PolyMesh with per-vertex skin data
  ==========================================================*/

  class SPolyMeshTraits
  {
  public:
    class Vertex; class HalfEdge; class Edge; class Face;

    class Vertex : public VertexBase <SPolyMeshTraits,PolyMesh> {
      DECLARE_SUBCLASS( Vertex, PolyMesh::Vertex ); DECLARE_END;
    public:
      Uint32 boneIndex [4];
      Float boneWeight [4];
    };
    
    class HalfEdge : public HalfEdgeBase <SPolyMeshTraits,PolyMesh> {
      DECLARE_SUBCLASS( HalfEdge, PolyMesh::HalfEdge ); DECLARE_END;
    };
    
    class Edge : public EdgeBase <SPolyMeshTraits,PolyMesh>  {
      DECLARE_SUBCLASS( Edge, PolyMesh::Edge ); DECLARE_END;
    };
    
    class Face : public FaceBase <SPolyMeshTraits,PolyMesh> {
      DECLARE_SUBCLASS( Face, PolyMesh::Face ); DECLARE_END;
    };
  };

  class SPolyMesh : public MeshBase <SPolyMeshTraits,PolyMesh>
  {
    DECLARE_SUBCLASS( SPolyMesh, PolyMesh ); DECLARE_END;
  };


  /*
  ==========================================================
  SkinTriMesh - triangular mesh export from 3DSMax
  ==========================================================*/

  class SkinTriMeshTraits
  {
  public:
    struct Vertex
    {
      Vector2 texcoord;
      Vector3 normal;
      Vector3 point;
      Uint32 boneIndex[4];
      Float32 boneWeight[4];
    };
  };

  class SkinTriMesh : public TriMeshBase <SkinTriMeshTraits, TriMesh>
  {
    DECLARE_SERIAL_SUBCLASS( SkinTriMesh, TriMesh );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;

  public:
    
    SkinTriMesh (SerializeManager *sm) : TriMeshBase <SkinTriMeshTraits, TriMesh> (sm) {}
    SkinTriMesh () {}
    
  protected:
    
    virtual void vertexFromPoly (PolyMesh::Vertex *polyVert,
                                 PolyMesh::VertexNormal *polyNormal,
                                 TexMesh::Vertex *texVert);
  };


  /*
  ==========================================================
  SkinMesh - generic mesh export that allows even PolyMesh
  to be reconstructed *after* exporting from 3DSMax
  ==========================================================*/

  struct SkinVertex
  {
    Vector3 point;
    Uint32  boneIndex[4];
    Float32 boneWeight[4];
  };
  
  struct SkinFace
  {
    Int32 numCorners;
    Int32 smoothGroups;
    Int32 materialId;
  };
  
  class SkinMesh
  {
    DECLARE_SERIAL_CLASS( SkinMesh );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;
    
  public:
    
    ArrayList <SkinVertex> verts;
    ArrayList <SkinFace> faces;
    ArrayList <Uint32> indices;
    
    void serialize (void *sm)
    {
      ((SM*)sm)->objectVar (&verts);
      ((SM*)sm)->objectVar (&faces);
      ((SM*)sm)->objectVar (&indices);
    }
    
    SkinMesh (SM *sm) : verts(sm), faces(sm), indices(sm)
    {}
    
    SkinMesh ()
    {}
  };


}//namespace GE
#endif//__GESKINPOLYMESH_H
