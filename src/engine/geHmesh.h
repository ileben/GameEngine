#ifndef __GEHMESH_H
#define __GEHMESH_H

#include "util/geUtil.h"
#include "geResource.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{

  typedef LinkedList<void*>::Iterator ListHandle;
  

  /*-----------------------------------------
  Half-edge mesh class
  ------------------------------------------*/

  class GE_API_ENTRY HMesh : public Resource
  {
    DECLARE_SUBCLASS (HMesh, Resource); DECLARE_END;
    
  public:

    //forward-declare base data types
    class Vertex;
    class HalfEdge;
    class Edge;
    class Face;

    //data and adjacency iterators
    #include "geHmeshDataiter.h"
    #include "geHmeshAdjiter.h"

    //a helper for various operations
    union ItemTag {
      void* ptr;
      int id;
      ItemTag() : id(0) {};
    };

    /*-----------------------------------------
    Vertex class
    ------------------------------------------*/

    class GE_API_ENTRY Vertex
    {
      DECLARE_CLASS (Vertex); DECLARE_END;

    public:
      ItemTag tag;
      ListHandle handle;
      HalfEdge *hedge;
      bool valid;
      
      Vertex(): valid(true), hedge(NULL) {}
      virtual ~Vertex() {}
           
      INLINE HalfEdge* outHedge() { return hedge; }
      HalfEdge* outHedgeTo(Vertex *v);
      int degree();
    };
    
    /*-----------------------------------------
    HalfEdge class
    ------------------------------------------*/

    class GE_API_ENTRY HalfEdge
    {
      DECLARE_CLASS (HalfEdge); DECLARE_END;

    public:
      ItemTag tag;
      ListHandle handle;
      HalfEdge *twin;
      HalfEdge *next;
      HalfEdge *prev;
      Vertex *vert;
      Edge *edge;
      Face *face;
      bool valid;
      
      HalfEdge(): valid(true),
        twin(NULL), next(NULL), prev(NULL),
        vert(NULL), edge(NULL), face(NULL) {}
      virtual ~HalfEdge() {}
      
      INLINE Vertex* dstVertex() { return vert; }
      INLINE Vertex* srcVertex() { return twin->vert; }
      INLINE HalfEdge* twinHedge() { return twin; }
      INLINE HalfEdge* nextHedge() { return next; }
      INLINE HalfEdge* prevHedge() { return prev; }
      INLINE Edge* fullEdge() { return edge; }
      INLINE Face* parentFace() { return face; }

      int loopLength();
      bool isLoopTriangle();
      bool isLoopEqual(HalfEdge *l);
      Vertex* oppositeVertex();
    };

    /*-----------------------------------------
    Edge class
    ------------------------------------------*/
    
    class GE_API_ENTRY Edge
    {
      DECLARE_CLASS (Edge); DECLARE_END;

    public:
      ItemTag tag;
      ListHandle handle;
      HalfEdge *hedge;
      bool valid;
      
      Edge(): valid(true),
        hedge(NULL) {}
      virtual ~Edge() {}
      
      INLINE Vertex* vertex1() { return hedge->vert; }
      INLINE Vertex* vertex2() { return hedge->twin->vert; }
      INLINE HalfEdge* hedge1() { return hedge; }
      INLINE HalfEdge* hedge2() { return hedge->twin; }
    };
    
    /*-----------------------------------------
    Face class
    ------------------------------------------*/

    class GE_API_ENTRY Face
    {
      DECLARE_CLASS (Face); DECLARE_END;
      
    public:
      ItemTag tag;
      ListHandle handle;
      HalfEdge *hedge;
      bool valid;

      Face(): valid(true),
        hedge(NULL) {}
      virtual ~Face() {}
      
      INLINE HalfEdge* firstHedge() { return hedge; }
      HalfEdge* hedgeTo(Vertex* v);
      int vertexCount();
      bool isTriangle();
    };

    /*-----------------------------------------------
    Classes to be used when entities are constructed
    ------------------------------------------------*/
    
  private:
    ClassPtr classVertex;
    ClassPtr classHalfEdge;
    ClassPtr classEdge;
    ClassPtr classFace;
    
  public:
    void setClasses(ClassPtr cnVertex, ClassPtr cnHedge,
                    ClassPtr cnEdge, ClassPtr cnFace);
    
    /*------------------------------------------------
    Main data collections, holding all the entities
    that define the mesh structure.
    -------------------------------------------------*/

    LinkedList<void*> verts;
    LinkedList<void*> hedges;
    LinkedList<void*> edges;
    LinkedList<void*> faces;
    
    /*---------------------------------------------------
    The entities removed as a result of certain mesh
    operations (e.g. WeldVertices) are marked invalid
    and temporarily stored into invalidation collections
    rather then being immediately deleted from memory.
    This allows user to get rid of its own pointers to
    invalid entities after invoking mesh operations
    ----------------------------------------------------*/

    LinkedList<void*> invalid_verts;
    LinkedList<void*> invalid_hedges;
    LinkedList<void*> invalid_edges;
    LinkedList<void*> invalid_faces;

    /*--------------------------------------------
    Constructor
    ---------------------------------------------*/

  public:

    HMesh()
    {
      classVertex = Class(Vertex);
      classHalfEdge = Class(HalfEdge);
      classEdge = Class(Edge);
      classFace = Class(Face);
    }
    
    /*--------------------------------------------
    Counters return numbers of a type of entity
    currently in the mesh structure.
    ---------------------------------------------*/

    INLINE int vertexCount() {return verts.size();}
    INLINE int hedgeCount() {return hedges.size();}
    INLINE int edgeCount() {return edges.size();}
    INLINE int faceCount() {return faces.size();}
    
    /*--------------------------------------------
    Entity insertion and removal functions.
    These make sure that entities have proper
    handles to list nodes for removal and
    store them into invalidation lists when
    removed by mesh operations.
    ---------------------------------------------*/

  protected:

    virtual void insertVert(Vertex *v);
    virtual void insertHalfEdge(HalfEdge *he);
    virtual void insertEdge(Edge *e);
    virtual void insertFace(Face *f);
    
    virtual ListHandle deleteVert(Vertex *v);
    virtual ListHandle deleteHalfEdge(HalfEdge *he);
    virtual ListHandle deleteEdge(Edge *e);
    virtual ListHandle deleteFace(Face *f);
    virtual void deleteEdgeWhole(Edge *e);

    /*----------------------------------------------
    Finally deletes the invalidated mesh entities
    which were removed by mesh operations
    -----------------------------------------------*/

  public:    
    void clearInvalid();
    
    /*-----------------------------------------------
    Deletes all the mesh entities and destroys the
    entire mesh structure.
    ------------------------------------------------*/

    void clear();

    /*--------------------------------------
    Destructor
    ---------------------------------------*/
    
    virtual ~HMesh() {
      clear();
    }
    
    /*------------------------------------------------------------
    Transfers the mesh entities from given HMesh to this one
    adding them to existing mesh structure. Just the pointers
    to entities are transfered without actually copying their
    data. The given mesh structured is empty after the function
    returns. This is the fastest way to merge two meshes.
    -------------------------------------------------------------*/

    void mergeWith(HMesh *mesh);

    /*--------------------------------------------------------------
    Stores the mesh structure into a chunk of memory independent
    of its actual memory layout by swapping the pointer
    references with indexed entity id's. The resulting mesh
    representation is safe for transfer across network.
    --------------------------------------------------------------*/

    void* serialize(int *outSize=NULL);

    /*-----------------------------------------------------------------
    Unpacks the mesh structure from given serialized representation
    back into pointer-referenced representation for fast operations
    on mesh. The unserialized entities are added to the existing
    mesh structure rather than replacing it.
    -----------------------------------------------------------------*/
    
    void deserialize(void *data);
    
  private:
    
    void linkOverPerpendicular(HalfEdge *he);
    void linkOverLinear(HalfEdge *he);
    void transferIncomingEdges(HalfEdge *first, HalfEdge *last, Vertex *target);
    void transferIncomingEdges(Vertex *from, Vertex *to);
    void connectManifolds(LinkedList<HalfEdge*> *outManifolds);
    void mergeEdges(HalfEdge *he1, HalfEdge *he2);

  public:
    
    Vertex* addVertex();
    Face* addFace(Vertex **vertices, int count);
    void removeFace(Face *face);
    bool removeEdge(Edge *edge);
    bool collapseEdge(Edge *edge);
    bool weldVertices(Vertex *vert1, Vertex *vert2);
    HalfEdge* findBoundary();
  };
  
  
  
  /*------------------------------------------------------------
  The following template classes act as a bridge between new
  derived versions of HMesh entity classes and the original
  ones. They insert overriden versions of basic mesh traversal
  and adjacency functions as wrappers around their base versions.
  The only difference is the overriden versions return the
  pointers of derived entity types instead of base ones. This
  makes the derived mesh classes easier to use, since any
  adjancency query invoked on a derived level classes returns
  the derived version of pointers as well.
  -------------------------------------------------------------*/

  template <class Derived, class Base> class VertexBase : public Base::Vertex
  {
  public:
    INLINE typename Derived::HalfEdge* outHedge() {
      return (typename Derived::HalfEdge*) HMesh::Vertex::outHedge(); }
    INLINE typename Derived::HalfEdge* outHedgeTo( typename Derived::Vertex *v) {
      return (typename Derived::HalfEdge*) HMesh::Vertex::outHedgeTo( v ); }
  };

  template <class Derived, class Base> class HalfEdgeBase : public Base::HalfEdge
  {
  public:
    INLINE typename Derived::Vertex* dstVertex() {
      return (typename Derived::Vertex*) HMesh::HalfEdge::dstVertex(); }
    INLINE typename Derived::Vertex* srcVertex() {
      return (typename Derived::Vertex*) HMesh::HalfEdge::srcVertex(); }
    INLINE typename Derived::HalfEdge* twinHedge() {
      return (typename Derived::HalfEdge*) HMesh::HalfEdge::twinHedge(); }
    INLINE typename Derived::HalfEdge* nextHedge() {
      return (typename Derived::HalfEdge*) HMesh::HalfEdge::nextHedge(); }
    INLINE typename Derived::HalfEdge* prevHedge() {
      return (typename Derived::HalfEdge*) HMesh::HalfEdge::prevHedge(); }
    INLINE typename Derived::Edge* fullEdge() {
      return (typename Derived::Edge*) HMesh::HalfEdge::fullEdge(); }
    INLINE typename Derived::Face* parentFace() {
      return (typename Derived::Face*) HMesh::HalfEdge::parentFace(); }
  };

  template <class Derived, class Base> class EdgeBase : public Base::Edge
  {
  public:
    INLINE typename Derived::Vertex* vertex1() {
      return (typename Derived::Vertex*) HMesh::Edge::vertex1(); }
    INLINE typename Derived::Vertex* vertex2() {
      return (typename Derived::Vertex*) HMesh::Edge::vertex2(); }
    INLINE typename Derived::HalfEdge* hedge1() {
      return (typename Derived::HalfEdge*) HMesh::Edge::hedge1(); }
    INLINE typename Derived::HalfEdge* hedge2() {
      return (typename Derived::HalfEdge*) HMesh::Edge::hedge2(); }
  };

  template <class Derived, class Base> class FaceBase : public Base::Face
  {
  public:
    INLINE typename Derived::HalfEdge* firstHedge() {
      return (typename Derived::HalfEdge*) HMesh::Face::firstHedge(); }
    INLINE typename Derived::HalfEdge* hedgeTo(typename Derived::Vertex *v) {
      return (typename Derived::HalfEdge*) HMesh::Face::hedgeTo(v); }
  };


}//namespace GE
#pragma warning(pop)
#endif //__GE_HMESH_H
