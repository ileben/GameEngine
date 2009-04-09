#ifndef __GESTATICMESH_H
#define __GESTATICMESH_H

#include "util/geUtil.h"
#include "geVectors.h"
#include "geResource.h"
#include "geMaterial.h"
#include "gePolyMesh.h"
#include "geTexMesh.h"
#include "geShader.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  /*
  --------------------------------------------------------
  VertexFormat defines the type and position of the data
  within the vertex structure
  --------------------------------------------------------*/

  struct VFMember
  {
    ShaderData::Enum data;
    DataUnit unit;
    UintSize offset;
    CharString attribName;
    DataUnit attribUnit;
    bool attribNorm;
    Int32 attribID;

    VFMember () {};
    VFMember (ShaderData::Enum newData,
              DataUnit newUnit,
              UintSize newOffset,
              CharString newAttribName = "",
              DataUnit newAttribUnit = DataUnit(),
              bool newAttribNorm = false)
    {
      data = newData;
      unit = newUnit;
      offset = newOffset;
      attribUnit = newAttribUnit;
      attribName = newAttribName;
      attribNorm = newAttribNorm;
    }
  };

  class VFormat
  { public:
    UintSize size;
    ArrayList<VFMember> members;
    void addMember (const VFMember &m) { members.pushBack( m ); }
    VFormat( UintSize newSize ) { size = newSize; }
  };


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

    class VertexFormat : public VFormat { public:
      VertexFormat() : VFormat(sizeof(Vertex))
      {
        addMember( VFMember( ShaderData::TexCoord, DataUnit::Vec2, 0 ) );
        addMember( VFMember( ShaderData::Normal,   DataUnit::Vec3, sizeof(Vector2) ) );
        addMember( VFMember( ShaderData::Coord,    DataUnit::Vec3, sizeof(Vector2)+sizeof(Vector3) ) );
      }
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
    typedef TriMeshTraits::VertexFormat VertexFormat;
    virtual VFormat* getVertexFormat() {
      static VertexFormat vertexFormat;
      return &vertexFormat; }
    
    struct IndexGroup
    {
      MaterialID materialID;
      VertexID   start;
      VertexID   count;
    };
    
    //Mesh data
    GenericArrayList data;
    ArrayList <Uint32> indices;
    ArrayList <IndexGroup> groups;
    
    //Drawing data
    Uint32 dataVBO;
    Uint32 indexVBO;
    bool isOnGpu;

    
  protected:

    virtual bool isPolyVertexEqual (PolyMesh::Vertex *polyVert,
                                    PolyMesh::HalfEdge *polyHedge1,
                                    PolyMesh::HalfEdge *polyHedge2);
    
    virtual void vertexFromPoly (PolyMesh::Vertex *polyVert,
                                 PolyMesh::HalfEdge *polyHedge,
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
    { isOnGpu = false; }
    
    TriMesh (Uint32 vertexSize) : data (vertexSize, NULL)
    { isOnGpu = false; }
    
    TriMesh () : data (sizeof(Vertex), NULL)
    { isOnGpu = false; }
    
    void addVertex (void *data);
    void addFaceGroup (MaterialID matID);
    void addFace (VertexID v1, VertexID v2, VertexID v3);
    void fromPoly (PolyMesh *m, TexMesh *uv);
    void toSubMesh (TriMesh *m);

    VertexID getCornerIndex (UintSize group, UintSize face, UintSize corner);
    Vertex* getVertex (UintSize index);
    UintSize getVertexCount ();
    UintSize getFaceCount ();
    UintSize getGroupFaceCount (UintSize group);

    void sendToGpu ();
  };

  /*
  ----------------------------------------------------------------
  Glue base class
  ----------------------------------------------------------------*/
  
  template <class Derived, class Base> class TriMeshBase : public Base
  {
  public:

    typedef typename Derived::Vertex Vertex;
    typedef typename Derived::VertexFormat VertexFormat;
    virtual VFormat* getVertexFormat() {
      static VertexFormat vertexFormat;
      return &vertexFormat; }

    TriMeshBase (SerializeManager *sm) : Base (sm)
    {}

    TriMeshBase () : Base (sizeof(typename Derived::Vertex))
    {}

    INLINE void addVertex (typename Derived::Vertex *data) {
      Base::addVertex( (void*) data );
    }

    INLINE typename Derived::Vertex* getVertex (UintSize index) {
      return (typename Derived::Vertex*) Base::getVertex( index ); }
  };

  /*
  ----------------------------------------------------------------
  Algorithm that copies a part of TriMesh into another sub-mesh
  ----------------------------------------------------------------*/

  typedef std::map<VertexID,VertexID> Super2SubMap;
  typedef Super2SubMap::iterator Super2SubIter;

  struct SubMeshInfo
  {
    TriMesh *mesh;
    VertexID nextVertexID;
    Super2SubMap super2subMap;
    SubMeshInfo() : nextVertexID(0), mesh(NULL) {}
  };

  class SuperToSubMesh
  {
  private:
    TriMesh *super;
    ArrayList<SubMeshInfo> subs;

  protected:
    virtual UintSize subMeshForFace (UintSize superGroup, UintSize superFace);
    virtual void newSubFace (UintSize subMeshID, VertexID subID1, VertexID subID2, VertexID subID3);
    virtual void newSubVertex (UintSize subMeshID, VertexID superID);
    virtual TriMesh* newSubMesh (UintSize subMeshID);

  public:
    SuperToSubMesh (TriMesh *superMesh);
    TriMesh* getSuperMesh() { return super; }
    TriMesh* getSubMesh (UintSize subMeshID) { return subs[ subMeshID ].mesh; }
    UintSize getSubMeshCount () { return subs.size(); }
    std::pair<bool,VertexID> getSubVertexID (UintSize subMeshID, VertexID superID);
    virtual void split();
  };


}//namespace GE
#pragma warning(pop)
#endif // __GESTATICMESH_H
