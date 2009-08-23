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

    typedef PolyMesh::VertexNormal VertexNormal;
    typedef PolyMesh::Triangle Triangle;
  };

  class SPolyMesh : public PolyMeshBase <SPolyMeshTraits,PolyMesh>
  {
    DECLARE_SUBCLASS( SPolyMesh, PolyMesh ); DECLARE_END;
  };

  /*
  ==========================================================
  Tri mesh with skin data
  ==========================================================*/

  class SkinVertex
  {
  public:
    Vector2 *texcoord;
    Vector3 *normal;
    Vector3 *coord;
    Uint32 *jointIndex;
    Float32 *jointWeight;

    DECLARE_CLASS( SkinVertex );
    DECLARE_MEMBER_DATA( texcoord, new BindTarget( ShaderData::TexCoord ) );
    DECLARE_MEMBER_DATA( normal, new BindTarget( ShaderData::Normal ) );
    DECLARE_MEMBER_DATA( coord, new BindTarget( ShaderData::Coord ) );
    DECLARE_MEMBER_DATA( jointIndex, new BindTarget( "jointIndex" ) );
    DECLARE_MEMBER_DATA( jointWeight, new BindTarget( "jointWeight" ) );
    DECLARE_END;
  };

  class SkinTriMesh : public TriMesh
  {
    DECLARE_SERIAL_SUBCLASS( SkinTriMesh, TriMesh );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;

  public:

    Uint32 mesh2skinSize;
    Uint32 mesh2skinMap[24];
    VertexBinding <SkinVertex> binding;
    
    void serialize (void *sm)
    {
      TriMesh::serialize( sm );
      //for (int i=0; i<24; ++i)
        //((SM*)sm)->dataVar( &mesh2skinMap[i] );
    }

    SkinTriMesh (SerializeManager *sm) : TriMesh (sm) {}
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
