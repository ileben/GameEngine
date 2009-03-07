#ifndef __GESTATICMESH_H
#define __GESTATICMESH_H

#include "util/geUtil.h"
#include "geVectors.h"
#include "geResource.h"
#include "geMaterial.h"
#include "gePolyMesh.h"
#include "geTexMesh.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  /*
  ---------------------------------------------------------
  Triangular mesh is highly optimized for rendering.
  Since OpenGL doesn't allow separate indices for vertex
  and texture/color/normal arrays, certain information has
  to be duplicated many times in the respective array in
  case another varies across faces adjacent to a vertex.
  ---------------------------------------------------------*/
  
  typedef Uint32 VertexID;
  
  class TriMeshTraits
  {
  public:
    struct Vertex
    {
      Vector2 texcoord;
      Vector3 normal;
      Vector3 point;
    };
  };
  
  class TriMesh : public Resource
  {
    friend class Renderer;
    DECLARE_SERIAL_SUBCLASS( TriMesh, Resource );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;
    
  public:

    typedef TriMeshTraits::Vertex Vertex;
    
    struct IndexGroup
    {
      MaterialID materialID;
      VertexID   start;
      VertexID   count;
    };
    
    GenericArrayList data;
    ArrayList <Uint32> indices;
    ArrayList <IndexGroup> groups;
    
  protected:
    
    virtual void vertexFromPoly (PolyMesh::Vertex *polyVert,
                                 PolyMesh::VertexNormal *polyNormal,
                                 TexMesh::Vertex *texVert);
    
    virtual void faceFromPoly (PolyMesh::Face *polyFace);
    
  public:
    
    void serialize (void *sm)
    {
      ( (SM*)sm )->objectVar( &data );
      ( (SM*)sm )->objectVar( &indices );
      ( (SM*)sm )->objectVar( &groups );
    }
    
    TriMesh (SerializeManager *sm) : data(sm), indices(sm), groups(sm)
    {}
    
    TriMesh (Uint32 vertexSize) : data (vertexSize, NULL)
    {}
    
    TriMesh () : data (sizeof(Vertex), NULL)
    {}
    
    void addVertex (void *data);
    void addFaceGroup (MaterialID matID);
    void addFace (VertexID v1, VertexID v2, VertexID v3);
    void fromPoly (PolyMesh *m, TexMesh *uv);
    Vertex* getVertex (int index);
  };
  
  template <class Derived, class Base> class TriMeshBase : public Base
  {
  public:

    typedef typename Derived::Vertex Vertex;

    TriMeshBase (SerializeManager *sm) : Base (sm)
    {}

    TriMeshBase () : Base (sizeof(Derived::Vertex))
    {}

    INLINE typename Derived::Vertex* getVertex (int index) {
      return (typename Derived::Vertex*) Base::getVertex( index ); }
  };

}//namespace GE
#pragma warning(pop)
#endif // __GESTATICMESH_H
