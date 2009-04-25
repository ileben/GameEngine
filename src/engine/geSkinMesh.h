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

    class VertexFormat : public VFormat { public:
      VertexFormat() : VFormat(sizeof(Vertex))
      {
        addMember( VFMember( ShaderData::TexCoord, DataUnit::Vec2, sizeof(Vector2) ));
        addMember( VFMember( ShaderData::Normal, DataUnit::Vec3, sizeof(Vector3) ));
        addMember( VFMember( ShaderData::Coord, DataUnit::Vec3, sizeof(Vector3) ));
        addMember( VFMember( ShaderData::Attribute, DataUnit::UVec4, sizeof(Uint32)*4,
                             "boneIndex", DataUnit::Vec4 ));
        addMember( VFMember( ShaderData::Attribute, DataUnit::Vec4, sizeof(Float32)*4,
                             "boneWeight", DataUnit::Vec4 ));
      }
    };
  };

  class SkinTriMesh : public TriMeshBase <SkinTriMeshTraits, TriMesh>
  {
    DECLARE_SERIAL_SUBCLASS( SkinTriMesh, TriMesh );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;

  public:

    Uint32 mesh2skinSize;
    Uint32 mesh2skinMap[24];
    
    void serialize (void *sm)
    {
      TriMesh::serialize( sm );
      //for (int i=0; i<24; ++i)
        //((SM*)sm)->dataVar( &mesh2skinMap[i] );
    }

    SkinTriMesh (SerializeManager *sm) : TriMeshBase <SkinTriMeshTraits, TriMesh> (sm) {}
    SkinTriMesh () : mesh2skinSize(0) {}
    
  protected:
    
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
  ===============================================
  Tri mesh with skin and tangent data
  ===============================================*/
/*
  class SkinTanTriMeshTraits
  {
  public:
    struct Vertex
    {
      Vector2 texcoord;
      Vector3 normal;
      Vector3 point;
      Uint32 boneIndex[4];
      Float32 boneWeight[4];
      Vector3 tangent;
      Vector3 bitangent;
    };

    class VertexFormat : public VFormat { public:
      VertexFormat() : VFormat(sizeof(Vertex))
      {
        addMember( VFMember( ShaderData::TexCoord, DataUnit::Vec2, sizeof(Vector2) ) );
        addMember( VFMember( ShaderData::Normal,   DataUnit::Vec3, sizeof(Vector3) ) );
        addMember( VFMember( ShaderData::Coord,    DataUnit::Vec3, sizeof(Vector3) ) );
        addMember( VFMember( ShaderData::Attribute, DataUnit::UVec4, sizeof(Uint32)*4,
                             "boneIndex", DataUnit::Vec4 ));
        addMember( VFMember( ShaderData::Attribute, DataUnit::Vec4, sizeof(Float32)*4,
                             "boneWeight", DataUnit::Vec4 ));
        addMember( VFMember( ShaderData::Custom,   DataUnit::Vec3, sizeof(Vector3),
                             "Tangent", DataUnit::Vec3 ));
        addMember( VFMember( ShaderData::Custom,   DataUnit::Vec3, sizeof(Vector3),
                             "Bitangent", DataUnit::Vec3 ));
      }
    };
  };

  class SkinTanTriMesh : public TriMeshBase <SkinTanTriMeshTraits, SkinTriMesh>
  {
    DECLARE_SERIAL_SUBCLASS( SkinTanTriMesh, SkinTriMesh );
    DECLARE_END;

  public:
    SkinTanTriMesh (SerializeManager *sm) : TriMeshBase <SkinTanTriMeshTraits, SkinTriMesh> (sm) {}
    SkinTanTriMesh () {}
  };*/



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
