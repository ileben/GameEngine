#ifndef __GEDYNAMICMESH_H
#define __GEDYNAMICMESH_H

#include "util/geUtil.h"
#include "geVectors.h"
#include "geHmesh.h"
#include "geMaterial.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{

  /*
  ========================================================
  Shading model determines whether face or per-vertex
  normals will be calculated.
  ======================================================*/

  namespace ShadingModel
  {
    enum Enum
    {
      Flat     = 1,
      Smooth   = 2
    };
  }

  /*
  ==========================================================
  Traits defining the entity classes for the polygonal mesh
  ==========================================================*/

  class PolyMeshTraits
  {
  public:

    class Vertex; class Edge; class HalfEdge; class Face; class VertexNormal;

    /*
    -----------------------------------------------------------
    Vertex holds the point in 3D space
    -----------------------------------------------------------*/

    class GE_API_ENTRY Vertex : public VertexBase <PolyMeshTraits,HMesh> {
      DECLARE_SUBCLASS (Vertex, HMesh::Vertex); DECLARE_END;
    public:
      Vector3 point;
    };

    /*
    ----------------------------------------------------------
    No special data per edge
    ----------------------------------------------------------*/

    class GE_API_ENTRY Edge : public EdgeBase <PolyMeshTraits,HMesh> {
      DECLARE_SUBCLASS (Edge, HMesh::Edge); DECLARE_END;
    };

    /*
    --------------------------------------------------------
    Half edge holds uvw coordinate and final normal for the
    target vertex with regard to its face. These can differ
    from one to another face for each vertex in case the
    adjacent faces are detached in the uvw map and/or the
    two faces are not in the same smoothing group
    -------------------------------------------------------*/

    class GE_API_ENTRY HalfEdge : public HalfEdgeBase <PolyMeshTraits,HMesh> {
      DECLARE_SUBCLASS (HalfEdge, HMesh::HalfEdge); DECLARE_END;
    public:
      VertexNormal *vnormal;
      HalfEdge() {vnormal = NULL;}
      INLINE VertexNormal* vertexNormal() { return vnormal; }
    };
    
    /*
    -------------------------------------------------
    Bits in the 32-bit field define which of the 32
    smoothing groups the face belongs to. MaterialID
    is an index into material's sub-material array.
    -------------------------------------------------*/

    class GE_API_ENTRY Face : public FaceBase <PolyMeshTraits,HMesh> {
      DECLARE_SUBCLASS (Face, HMesh::Face); DECLARE_END;
      friend class PolyMesh;
    private:
      Uint8 matId;
    public:
      Vector3 center;
      Vector3 normal;
      Uint32 smoothGroups;
      Face() { smoothGroups = 0; matId = 0; }
      Uint8 materialID() { return matId; }
    };

    /*
    -----------------------------------------------
    Smooth normals are dynamically allocated like
    the rest of the mesh entities and referenced
    by halfedges via pointers. This minimizes the
    memory space when many faces belong to same
    smoothing groups. When inserted into mesh,
    each new halfedge is set to point to a dummy
    normal, so it is still valid before normals
    get updated.
    ----------------------------------------------*/

    class VertexNormal
    {
    public:
      ItemTag tag;
      Vector3 coord;
      Vertex *vert;
      VertexNormal () {};
      VertexNormal (const Vector3 &c) {coord = c;}
    };
  };
  
  /*
  =============================================================
  PolyMesh is derived from a HMesh instantiation with custom
  traits to hold aditional data per each mesh entity. The fast
  adjacency queries allow for calculation of normals and other
  complex mesh operations.
  =============================================================*/

  class GE_API_ENTRY PolyMesh : public MeshBase <PolyMeshTraits,HMesh>
  {
    friend class TriMesh;
    friend class LoaderObj;
    friend class Renderer;
    DECLARE_SUBCLASS (PolyMesh, HMesh);
    DECLARE_END;
    
  public:

    typedef PolyMeshTraits::VertexNormal VertexNormal;
    #include "gePolyMeshIters.h"

  private:
    bool useSmoothGroups;
    VertexNormal dummyVertexNormal;
    DynamicArrayList<VertexNormal> vertexNormals;
    virtual void insertHalfEdge (HMesh::HalfEdge *he);
    
    /*
    -----------------------------------------------
    We keep track of all the material IDs used on
    this mesh so the renderer knows how many times
    to traverse it.
    ----------------------------------------------*/
    
  private:
    int facesPerMaterial [GE_MAX_MATERIAL_ID];
    LinkedList<MaterialID> materialsUsed;
    void addMaterialID (MaterialID m);
    void subMaterialID (MaterialID m);
    virtual void insertFace (HMesh::Face *f);
    virtual ListHandle deleteFace (HMesh::Face *f);


  public:

    PolyMesh();
    
    //When true causes smoothing groups to be taken
    //into account when calculating vertex normals
    void setUseSmoothGroups (bool enabled) {
      useSmoothGroups = enabled;
    }

    bool getUseSmoothGroups () {
      return useSmoothGroups;
    }

  private:

    void updateFaceNormal (Face *f);
    void updateVertNormalFlat (Vertex *v);
    void updateVertNormalSmooth (Vertex *v);
    void updateVertNormalGroups (Vertex *v);

  public:

    void triangulate ();
    void updateNormals (ShadingModel::Enum shadingModel);
    void setMaterialID (Face *f, MaterialID id);
  };

  template <class Derived, class Base> class PolyMeshBase : public MeshBase <Derived,Base>
  {
  public:
    typedef typename Derived::Vertex       Vertex;
    typedef typename Derived::HalfEdge     HalfEdge;
    typedef typename Derived::Edge         Edge;
    typedef typename Derived::Face         Face;
    typedef typename Derived::VertexNormal VertexNormal;
    #include "gePolyMeshIters.h"
  };


}//namespace GE
#pragma warning(pop)
#endif//__GEDYNAMICMESH_H
