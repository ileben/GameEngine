#ifndef __GESKINPOLYMESH_H
#define __GESKINPOLYMESH_H

#include "util/geUtil.h"
#include "math/geMath.h"
#include "core/geTriMesh.h"
#include "core/gePolyMesh.h"

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

    class Vertex : public VertexBase <SPolyMeshTraits,PolyMesh>
    {  
      CLASS( Vertex, PolyMesh::Vertex,
        8525e796,ef4c,406a,a610a0f58b1c8284 );

    public:
      Uint32 boneIndex [4];
      Float boneWeight [4];
    };
    
    class HalfEdge : public HalfEdgeBase <SPolyMeshTraits,PolyMesh>
    {
      CLASS( HalfEdge, PolyMesh::HalfEdge,
        4d55f281,ba03,40da,a6ed7ca5bf23acf0 );
    };
    
    class Edge : public EdgeBase <SPolyMeshTraits,PolyMesh>
    {
      CLASS( Edge, PolyMesh::Edge,
        1e36eede,e3ab,46c5,9a85e73b83d19aa5 );
    };
    
    class Face : public FaceBase <SPolyMeshTraits,PolyMesh>
    {
      CLASS( Face, PolyMesh::Face,
        f64f2b5f,db3e,42d0,b01fdca37f02833a );
    };

    typedef PolyMesh::VertexNormal VertexNormal;
    typedef PolyMesh::Triangle Triangle;
  };

  class SPolyMesh : public PolyMeshBase <SPolyMeshTraits,PolyMesh>
  {
    CLASS( SPolyMesh, PolyMesh,
      4b87e6ad,c85c,4863,93de87ef8577a609 );
  };

  /*
  ==========================================================
  Tri mesh with skin data
  ==========================================================*/

  struct SkinVertex
  {
    Vector2 *texcoord;
    Vector3 *normal;
    Vector3 *coord;
    Uint32 *jointIndex;
    Float32 *jointWeight;

    void bind (VertexBinding<SkinVertex> *b)
    {
      b->bind( &texcoord, ShaderData::TexCoord2 );
      b->bind( &normal, ShaderData::Normal );
      b->bind( &coord, ShaderData::Coord3 );
      b->bind( &jointIndex, ShaderData::JointIndex );
      b->bind( &jointWeight, ShaderData::JointWeight );
    }
  };

  class SkinTriMesh : public TriMesh
  {
    CLASS( SkinTriMesh, TriMesh,
      69dcb64f,c761,4069,870a002a3470a7e9 );

    virtual void serialize( Serializer *s, Uint v )
    {
      TriMesh::serialize( s,v );
      s->data( &mesh2skinSize );
      s->data( &mesh2skinMap );
    }

  public:

    Uint32 mesh2skinSize;
    Uint32 mesh2skinMap[24];
    VertexBinding <SkinVertex> binding;
    SkinTriMesh () : mesh2skinSize(0) {}
    
  protected:
    
    virtual void fromPoly (PolyMesh *m, TexMesh *um);
    virtual void vertexFromPoly (PolyMesh::Vertex *polyVert,
                                 PolyMesh::HalfEdge *polyHedge,
                                 TexMesh::Vertex *texVert);
  };

  /*
  -------------------------------------------------------------
  Algorithm that copies faces of a mesh into a sub mesh
  and remaps the bone indices until the bone limit is reached
  -------------------------------------------------------------*/

  typedef std::map<Uint32,Uint32> Skin2MeshMap;
  typedef Skin2MeshMap::iterator Skin2MeshIter;


  struct SkinSubMeshInfo
  {
    Uint32 nextBoneID;
    SkinTriMesh *mesh;
    Skin2MeshMap skin2meshMap;
    SkinSubMeshInfo() : nextBoneID(0) {}
  };

  class SkinSuperToSubMesh : public SuperToSubMesh
  {
  private:
    Uint32 maxBones;
    Uint32 subMeshCount;
    ArrayList<SkinSubMeshInfo> subs;
    VertexBinding<SkinVertex> binding;

  protected:
    std::pair<bool,Uint32> getSubBoneID ( UintSize subMeshID, Uint32 superID );
    virtual UintSize subMeshForFace (UintSize superGroup, UintSize superFace);
    virtual void newSubVertex (UintSize subMeshID, VertexID superID);
    virtual TriMesh* newSubMesh (UintSize subMeshID);

  public:
    SkinSuperToSubMesh (SkinTriMesh *superMesh) : SuperToSubMesh(superMesh) {}
    void splitByBoneLimit (UintSize maxBonesPerMesh);
  };

  /*
  ==========================================================
  /// Obsolete ///

  SkinMesh - generic mesh export that allows even PolyMesh
  to be reconstructed after loading from a file

  /// Obsolete ///
  ==========================================================*/
/*
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
*/

}//namespace GE
#endif//__GESKINPOLYMESH_H
