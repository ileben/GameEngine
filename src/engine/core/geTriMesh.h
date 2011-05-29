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
    CLASS( FormatMember, Object,
      2bbbf581,6115,4079,92d4cd4adca55145 );

    virtual void serialize( Serializer *s, Uint v )
    {
      Object::serialize( s,v );
      s->data( &data );
      s->data( &unit );
      s->data( &size );
      s->data( &offset );
      s->string( &attribName );
      s->data( &attribUnit );
      s->data( &attribNorm );
    }

  public:
    ShaderData::Enum data;
    DataUnit unit;
    UintSize size;
    UintSize offset;
    CharString attribName;
    DataUnit attribUnit;
    bool attribNorm;

    FormatMember () {}
    bool operator == (const FormatMember &other) const;
    void resolveKnownData();
  };

  class VertexFormat : public Object
  {
    CLASS( VertexFormat, Object,
      a9e2d1db,0f6c,4f31,a061acb2742da9bc );

    virtual void serialize( Serializer *s, Uint v )
    {
      Object::serialize( s,v );
      s->data( &size );
      s->objectArray( &members );
    }

  private:
    UintSize size;
    ArrayList <FormatMember> members;

  public:

    VertexFormat () { size = 0; }

    UintSize getByteSize() const;
    const ArrayList <FormatMember> * getMembers () const;
    VertexFormat& operator= (const VertexFormat &f);
    bool operator == (const VertexFormat &other) const;

    FormatMember* findMember (ShaderData::Enum data,
                              const CharString &attribName) const;

    void addMember (const FormatMember &m);

    void addMember (ShaderData::Enum newData);

    void addMember (DataUnit newUnit,
                    UintSize newSize,
                    CharString newAttribName = "",
                    DataUnit newAttribUnit = DataUnit(),
                    bool newAttribNorm = false);
  };

  /*
  -------------------------------------------------------------
  VertexBinding allows for strong-typed access to vertex data
  -------------------------------------------------------------*/

  template <class VertexType> class VertexBinding
  {
    struct BindTarget
    {
      void *vmember;
      UintSize foffset;
    };

    const VertexFormat *format;
    VertexType prototype;
    ArrayList< BindTarget > targets;

  public:

    void init (const VertexFormat *vertexFormat)
    {
      format = vertexFormat;
      targets.clear();
      prototype.bind( this );
    }

    VertexType& operator() (void *data)
    {
      for (UintSize t=0; t<targets.size(); ++t)
        *(void**)targets[t].vmember = Util::PtrOff( data, targets[t].foffset );
      return prototype;
    }

    void bind (void *vmember, ShaderData::Enum data, const CharString name="")
    {
      //Find the matching format member
      FormatMember *fmember = format->findMember( data, name );
      if (fmember != NULL)
      {
        //Bind prototype member to format member
        BindTarget target;
        target.vmember = vmember;
        target.foffset = fmember->offset;
        targets.pushBack( target );
      }
      else
      {
        //Init unmatched prototype members to NULL
        *(void**)vmember = NULL;
      }
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

  struct TriVertex
  {
    Vector2 *texcoord;
    Vector3 *normal;
    Vector3 *coord;

    void bind (VertexBinding<TriVertex> *b)
    {
      b->bind( &texcoord, ShaderData::TexCoord2 );
      b->bind( &normal, ShaderData::Normal );
      b->bind( &coord, ShaderData::Coord3 );
    }
  };

  class TriMesh : public Resource
  {
    CLASS( TriMesh, Resource,
      b8ad23bf,f64d,4bd3,8d527b2db7c428d2 );

    virtual void serialize( Serializer *s, Uint v )
    {
      Resource::serialize( s, v );
      if (s->loading())
      {
        s->object( &format );
        setFormat( format );
        s->dataArray( &data );
        s->dataArray( &indices );
        s->dataArray( &groups );
        s->data( &bbox );
      }
      else
      {
        s->object( &format );
        s->dataArray( &data );
        s->dataArray( &indices );
        s->dataArray( &groups );
        s->data( &bbox );
      }
    }
   
  public:

    friend class Renderer;
    
    struct IndexGroup
    {
      MaterialID materialID;
      VertexID   start;
      VertexID   count;
    };
    
    //Mesh data
    GenericArrayList data;
    ArrayList <VertexID> indices;
    ArrayList <IndexGroup> groups;
    BoundingBox bbox;
    
    //Drawing data
    Uint32 dataVBO;
    Uint32 indexVBO;
    bool isOnGpu;

    
  protected:

    VertexFormat format;
    VertexBinding <TriVertex> binding;

    virtual bool isPolyVertexEqual (
      PolyMesh::Vertex *polyVert,
      PolyMesh::HalfEdge *polyHedge1,
      PolyMesh::HalfEdge *polyHedge2 );
    
    virtual void vertexFromPoly (
      PolyMesh::Vertex *polyVert,
      PolyMesh::HalfEdge *polyHedge,
      TexMesh::Vertex *texVert );
    
    virtual void faceFromPoly (
      PolyMesh::Face *polyFace );
    
  public:
    
    TriMesh (const VertexFormat &f) : data(f.getByteSize())
    { isOnGpu = false; setFormat( f ); }
    
    TriMesh () : data(sizeof(Uint8))
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

    void updateBoundingBox();
    BoundingBox getBoundingBox();

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
