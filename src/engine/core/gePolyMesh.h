#ifndef __GEDYNAMICMESH_H
#define __GEDYNAMICMESH_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "geHmesh.h"
#include "geMaterial.h"
#include "geTexMesh.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{

  /*
  ======================================================
  Forward declarations
  ======================================================*/

  /*
  ========================================================
  Smooth metric defines the criteria for averaging the
  normals across adjacent faces.
  ======================================================*/

  namespace SmoothMetric
  {
    enum Enum
    {
      None  = 1,
      All   = 2,
      Face  = 3,
      Edge  = 4
    };
  }

  /*
  ==========================================================
  Traits defining the entity classes for the polygonal mesh
  ==========================================================*/

  class PolyMeshTraits
  {
  public:

    class Vertex; class Edge; class HalfEdge; class Face;
    class Triangle; class VertexNormal; class VertexTangent;

    /*
    -----------------------------------------------------------
    Vertex holds the point in 3D space
    -----------------------------------------------------------*/

    class Vertex : public VertexBase <PolyMeshTraits,HMesh>
    {  
      CLASS( PolyMeshTraits::Vertex, HMesh::Vertex,
        ce0336ca,33cf,4f59,97d11d6d77826aed );

    public:
      Vector3 point;
    };

    /*
    ----------------------------------------------------------
    Edge holds the information whether it is smooth or hard
    for edge-based normal calculation.
    ----------------------------------------------------------*/

    class Edge : public EdgeBase <PolyMeshTraits,HMesh>
    {
      CLASS( PolyMeshTraits::Edge, HMesh::Edge,
        81e7dcda,cd5f,475d,b72af28ceb4f2822 );

    public:
      bool isSmooth;
      Edge() : isSmooth( false ) {}
    };

    /*
    --------------------------------------------------------
    Half edge holds uvw coordinate and final normal for the
    target vertex with regard to its face. These can differ
    from one to another face for each vertex in case the
    adjacent faces are detached in the uvw map and/or the
    two faces are not in the same smoothing group
    -------------------------------------------------------*/

    class HalfEdge : public HalfEdgeBase <PolyMeshTraits,HMesh>
    {
      CLASS( PolyMeshTraits::HalfEdge, HMesh::HalfEdge,
        849dc863,7529,4c2b,a748058c8479d410 );

    public:
      VertexNormal *vnormal;
      VertexTangent *vtangent;
      HalfEdge() {vnormal = NULL; vtangent = NULL; }
      INLINE VertexNormal* vertexNormal() { return vnormal; }
      INLINE VertexTangent* vertexTangent() { return vtangent; }
    };
    
    /*
    -------------------------------------------------
    Bits in the 32-bit field define which of the 32
    smoothing groups the face belongs to. MaterialID
    is an index into material's sub-material array.
    -------------------------------------------------*/

    class Face : public FaceBase <PolyMeshTraits,HMesh>
    {
      CLASS( PolyMeshTraits::Face, HMesh::Face,
        a384c0b7,48e3,4a8e,9388843322a63390 );

    private:
      friend class PolyMesh;
      MaterialID matId;
      Triangle *triangle;

    public:
      Vector3 center;
      Vector3 normal;
      Vector3 tangent;
      Vector3 bitangent;
      Uint32 smoothGroups;
      MaterialID materialID() { return matId; }
      Triangle* firstTriangle() { return triangle; }
      Face() { smoothGroups = 0; matId = 0; triangle = NULL; }
    };

    class Triangle : public Object
    {
      CLASS( PolyMeshTraits::Triangle, Object,
        67c50ff4,1c93,44e9,a6782780c5ac49fe );

    public:
      Triangle* next;
      HalfEdge* hedges[3];
      Vertex* vertex (int index) { return hedges[index]->dstVertex(); }
      HalfEdge* hedgeToVertex (int index) { return hedges[index]; }
      Triangle* nextTriangle() { return next; }
      Triangle() { next = NULL; }
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

    class VertexTangent
    {
    public:
      Vector3 coord;
      Vector3 bicoord;
      VertexTangent () {};
      VertexTangent (const Vector3 &c) {coord = c;}
      VertexTangent (const Vector3 &c, const Vector3 &b) {coord = c; bicoord = b;}
    };
  };
  
  /*
  =============================================================
  PolyMesh is derived from a HMesh instantiation with custom
  traits to hold aditional data per each mesh entity. The fast
  adjacency queries allow for calculation of normals and other
  complex mesh operations.
  =============================================================*/

  class PolyMesh : public MeshBase <PolyMeshTraits,HMesh>
  {
    CLASS( PolyMesh, HMesh,
      3810ed9f,68b8,49b0,9cc38e89c6507ac2 );

    friend class TriMesh;
    friend class LoaderObj;
    friend class Renderer;
    
  public:

    typedef PolyMeshTraits::Triangle Triangle;
    typedef PolyMeshTraits::VertexNormal VertexNormal;
    typedef PolyMeshTraits::VertexTangent VertexTangent;
    #include "gePolyMeshIters.h"

  private:

    DynamicArrayList<Triangle> triangles;

    VertexNormal dummyVertexNormal;
    VertexTangent dummyVertexTangent;
    DynamicArrayList<VertexNormal> vertexNormals;
    DynamicArrayList<VertexTangent> vertexTangents;
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

  private:

    void updateFaceNormal (Face *f);
    void updateFaceTangent (Face *f, TexMesh::Face *tf);

    void updateVertNormalFlat (Vertex *v);
    void updateVertNormalSmooth (Vertex *v);
    void updateVertNormalGroups (Vertex *v);
    void updateVertNormalEdges (Vertex *v);
    void updateVertTangent (Vertex *v, TexMesh::Vertex *tv);

  public:

    PolyMesh();

    void setMaterialID (Face *f, MaterialID id);
    void updateNormals (SmoothMetric::Enum metric = SmoothMetric::None);
    void updateTangents (TexMesh *texMesh);

    void triangulate ();
    void clearTriangles();
    void addTriangle (Face *f, HalfEdge *h1, HalfEdge *h2, HalfEdge *h3);
  };

  template <class Derived, class Base> class PolyMeshBase : public MeshBase <Derived,Base>
  {
  public:
    typedef typename Derived::Vertex        Vertex;
    typedef typename Derived::HalfEdge      HalfEdge;
    typedef typename Derived::Edge          Edge;
    typedef typename Derived::Face          Face;
    typedef typename Derived::Triangle      Triangle;
    typedef typename Derived::VertexNormal  VertexNormal;
    #include "gePolyMeshIters.h"
  };


}//namespace GE
#pragma warning(pop)
#endif//__GEDYNAMICMESH_H
