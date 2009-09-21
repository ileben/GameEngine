#ifndef __GESTATICMESH_H
#define __GESTATICMESH_H

#include "util/geUtil.h"
#include "math/geMath.h"
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

  class FormatMember : public Object
  {
    DECLARE_SERIAL_SUBCLASS( FormatMember, Object );
    DECLARE_DATAVAR( data );
    DECLARE_DATAVAR( unit );
    DECLARE_DATAVAR( size );
    DECLARE_DATAVAR( offset );
    DECLARE_OBJVAR( attribName );
    DECLARE_DATAVAR( attribUnit );
    DECLARE_DATAVAR( attribNorm );
    DECLARE_END;

  public:
    ShaderData::Enum data;
    DataUnit unit;
    UintSize size;
    UintSize offset;
    CharString attribName;
    DataUnit attribUnit;
    bool attribNorm;

    FormatMember () {}
    FormatMember (SM *sm) : attribName(sm) {}
  };

  class VertexFormat : public Object
  {
    DECLARE_SERIAL_SUBCLASS( VertexFormat, Object );
    DECLARE_DATAVAR( size );
    DECLARE_OBJVAR( members );
    DECLARE_END;

  private:
    UintSize size;
    ObjArrayList <FormatMember> members;

  public:

    VertexFormat (SM *sm) : members (sm) {}
    VertexFormat () { size = 0; }

    UintSize getByteSize() const;
    const ObjArrayList <FormatMember> * getMembers () const;
    VertexFormat& operator= (const VertexFormat &f);

    FormatMember* findMember (ShaderData::Enum data,
                              const CharString &attribName) const;

    void addMember (ShaderData::Enum newData,
                    DataUnit newUnit,
                    UintSize newSize,
                    CharString newAttribName = "",
                    DataUnit newAttribUnit = DataUnit(),
                    bool newAttribNorm = false);
  };

  /*
  -------------------------------------------------------------
  VertexBinding allows for strong-typed access to vertex data
  -------------------------------------------------------------*/

  class BindTarget
  {
  public:
    ShaderData::Enum type;
    CharString name;

    BindTarget (ShaderData::Enum t) {
      type = t;
    }

    BindTarget (const CharString &n) {
      type = ShaderData::Attribute;
      name = n;
    }
  };

  template <class VertexType> class VertexBinding
  {
    struct Binding
    {
      MemberPtr member;
      UintSize offset;
    };

    VertexType prototype;
    ArrayList <Binding> bindings;

  public:

    void init (const VertexFormat *vertexFormat)
    {
      //Get vertex class and format members
      const ObjArrayList< FormatMember > *fmembers = vertexFormat->getMembers();
      const MTable *vmembers = Class(VertexType)->getMembers();
      Binding binding;

      //Walk class members
      for (UintSize m=0; m<vmembers->size(); ++m)
      {
        //Get member bind target
        MemberPtr vmember = vmembers->at( m );
        BindTarget *target = (BindTarget*) vmember->data;

        //Find the matching format member
        FormatMember *fmember = vertexFormat->findMember ( target->type, target->name );
        if (fmember != NULL)
        {
          //Bind vertex to format member
          binding.member = vmember;
          binding.offset = fmember->offset;
          bindings.pushBack( binding );
        }
        else
        {
          //Init unmatched vertex members to NULL
          void **pmember = (void**) vmember->getFrom( &prototype );
          *pmember = NULL;
        }
      }
    }

    VertexType& operator()(void *dataPtr)
    {
      //Set pointer offsets to match data at given base
      for (UintSize b=0; b<bindings.size(); ++b) {
        void **pmember = (void**) bindings[b].member->getFrom( &prototype );
        *pmember = Util::PtrOff( dataPtr, bindings[b].offset );
      }

      return prototype;
    }
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

  class TriVertex : public Object
  {
  public:

    Vector2 *texcoord;
    Vector3 *normal;
    Vector3 *coord;

    DECLARE_SUBCLASS( TriVertex, Object );
    DECLARE_MEMBER_DATA( texcoord, new BindTarget( ShaderData::TexCoord ) );
    DECLARE_MEMBER_DATA( normal, new BindTarget( ShaderData::Normal ) );
    DECLARE_MEMBER_DATA( coord, new BindTarget( ShaderData::Coord ) );
    DECLARE_END;
  };

  class TriMesh : public Resource
  {
    friend class Renderer;
    DECLARE_SERIAL_SUBCLASS( TriMesh, Resource );
    DECLARE_OBJVAR( format );
    DECLARE_OBJVAR( data );
    DECLARE_OBJVAR( indices );
    DECLARE_OBJVAR( groups );
    DECLARE_END;
    
  public:
    
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

    VertexFormat format;
    VertexBinding <TriVertex> binding;

    virtual bool isPolyVertexEqual (PolyMesh::Vertex *polyVert,
                                    PolyMesh::HalfEdge *polyHedge1,
                                    PolyMesh::HalfEdge *polyHedge2);
    
    virtual void vertexFromPoly (PolyMesh::Vertex *polyVert,
                                 PolyMesh::HalfEdge *polyHedge,
                                 TexMesh::Vertex *texVert);
    
    virtual void faceFromPoly (PolyMesh::Face *polyFace);
    
  public:
    
    TriMesh (SerializeManager *sm) : Resource(sm), format(sm), data(sm), indices(sm), groups(sm)
    { isOnGpu = false; }

    TriMesh (const VertexFormat &f) : data(f.getByteSize())
    { isOnGpu = false; setFormat( f ); }
    
    TriMesh ()
    { isOnGpu = false; setDefaultFormat(); }

    void setDefaultFormat();
    void setFormat( const VertexFormat &f);
    const VertexFormat* getFormat() { return &format; }
    
    void* addVertex ();
    void* addVertex (void *data);
    void addFaceGroup (MaterialID matID);
    void addFace (VertexID v1, VertexID v2, VertexID v3);
    virtual void fromPoly (PolyMesh *m, TexMesh *uv);
    void toSubMesh (TriMesh *m);

    VertexID getCornerIndex (UintSize group, UintSize face, UintSize corner);
    void* getVertex (UintSize index);
    UintSize getVertexCount ();
    UintSize getFaceCount ();
    UintSize getGroupFaceCount (UintSize group);

    void sendToGpu ();
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
